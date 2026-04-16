  #include <stdio.h>
  #include <stdlib.h>
  #include <stdint.h>
  #include <string.h>
  #include <time.h>
  #include <sys/ioctl.h>
  #include <net/if.h>
  #include <linux/mii.h>
  #include <linux/sockios.h>
  #include <unistd.h>

  #define PHY_ADDR 1

  // Registers to capture on link event
  static const struct {
      int reg;
      const char *name;
  } regs[] = {
      { 0,  "BMCR    " },
      { 1,  "BMSR    " },
      { 4,  "ANAR    " },
      { 5,  "ANLPAR  " },
      { 6,  "ANER    " },
      { 17, "MODE_STS" },
      { 29, "INT_SRC " },
      { 31, "SPECIAL " },
  };
  #define NREGS (sizeof(regs)/sizeof(regs[0]))

  static int read_phy(int fd, struct ifreq *ifr, int reg) {
      struct mii_ioctl_data *mii = (struct mii_ioctl_data *)&ifr->ifr_data;
      mii->phy_id = PHY_ADDR;
      mii->reg_num = reg;
      if (ioctl(fd, SIOCGMIIREG, ifr) < 0)
          return -1;
      return mii->val_out;
  }

  static void dump_regs(int fd, struct ifreq *ifr, const char *event) {
      struct timespec ts;
      clock_gettime(CLOCK_REALTIME, &ts);

      struct tm *tm = localtime(&ts.tv_sec);
      printf("\n=== %s at %02d:%02d:%02d.%06ld ===\n",
             event,
             tm->tm_hour, tm->tm_min, tm->tm_sec,
             ts.tv_nsec / 1000);

      for (int i = 0; i < NREGS; i++) {
          int val = read_phy(fd, ifr, regs[i].reg);
          if (val >= 0) {
              printf("  Reg %2d (%s): 0x%04x", regs[i].reg, regs[i].name, val);

              // Decode key bits
              if (regs[i].reg == 1) { // BMSR
                  printf("  [Link:%s AuNeg:%s]",
                         (val & 0x0004) ? "UP" : "DOWN",
                         (val & 0x0020) ? "Done" : "...");
              } else if (regs[i].reg == 31) { // SPECIAL
                  int spd = (val >> 2) & 0x7;
                  const char *speed = "???";
                  if (spd == 1) speed = "10H";
                  else if (spd == 5) speed = "10F";
                  else if (spd == 2) speed = "100H";
                  else if (spd == 6) speed = "100F";
                  printf("  [Speed:%s]", speed);
              } else if (regs[i].reg == 29) { // INT_SRC
                  printf("  [%s%s%s%s]",
                         (val & 0x0010) ? "LinkDn " : "",
                         (val & 0x0020) ? "RemFlt " : "",
                         (val & 0x0040) ? "ANcmpl " : "",
                         (val & 0x0080) ? "Energy " : "");
              }
              printf("\n");
          }
      }
      fflush(stdout);
  }

  int main(int argc, char *argv[]) {
      const char *ifname = argc > 1 ? argv[1] : "eth0";
      int fd = socket(AF_INET, SOCK_DGRAM, 0);
      if (fd < 0) { perror("socket"); return 1; }

      struct ifreq ifr;
      strncpy(ifr.ifr_name, ifname, IFNAMSIZ - 1);

      uint16_t prev_bmsr = 0xFFFF;
      unsigned long count = 0;

      printf("Monitoring %s for link events (Ctrl+C to stop)...\n", ifname);

      while (1) {
          int bmsr = read_phy(fd, &ifr, 1);
          if (bmsr < 0) {
              usleep(1000);
              continue;
          }

          uint16_t link = bmsr & 0x0004;
          uint16_t prev_link = prev_bmsr & 0x0004;

          if (prev_bmsr != 0xFFFF && link != prev_link) {
              dump_regs(fd, &ifr, link ? "LINK UP" : "LINK DOWN");
          }

          prev_bmsr = bmsr;
          count++;

          // Print poll rate every 10 seconds
          if ((count % 100000) == 0) {
              fprintf(stderr, "\r[polling: %lu reads]", count);
          }
      }

      close(fd);
      return 0;
  }
