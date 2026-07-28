#include "../../src/wifi/IskakINO_WifiPortal.h"
int main() {
    IskakINO_WifiPortal portal;
    portal.setDebug(false);
    portal.setPortalTimeout(60);
    (void)portal.lastError();
    return 0;
}
