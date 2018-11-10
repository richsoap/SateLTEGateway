#include <pjsua2.hpp>
#include <iostream>

using namespace pj;

class MyAccount: public Account {
    public:
        virtual void onRegState(OnRegStateParam &prm) {
            AccountInfo ai = getInfo();
            std::cout << (ai.regIsActive?"*** Register:": "*** Unregister:")
                << " code=" << prm.code << std::endl;
        }
};

int main() {
    Endpoint ep;
    ep.libCreate();

    EpConfig ep_cfg;
    ep.libInit(ep_cfg);

    TransportConfig tcfg;
    tcfg.port = 5060;
    try {
        ep.transportCreate(PJSIP_TRANSPORT_UDP, tcfg);
    }catch (Error &err) {
        std::cout << err.info() << std::endl;
        return 1;
    }

    ep.libStart();
    std::cout << "*** PJSUA2 STARTED ***" << std::endl;

    AccountConfig acfg;
    acfg.idUri = "sip:test@pjsip.org";
    acfg.regConfig.registrarUri = "sip:pjsip.org";
    AuthCredInfo cred("digest", "*", "test", 0, "secret");
    acfg.sipConfig.authCreds.push_back(cred);

    MyAccount *acc = new MyAccount;
    acc->create(acfg);


    pj_thread_sleep(10000);

    delete acc;

    return 0;

}
