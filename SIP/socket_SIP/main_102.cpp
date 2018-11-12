/* $Id: pjsua2_demo.cpp 5717 2017-12-19 01:45:37Z nanang $ */
/*
 * Copyright (C) 2008-2013 Teluu Inc. (http://www.teluu.com)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
#include <pjsua2.hpp>
#include <iostream>
#include <pj/file_access.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "socketmedia.h"
#include "socket_thread.h"


#define THIS_FILE 	"pjsua2_demo.cpp"

using namespace pj;

class MyAccount;

class MyCall : public Call
{
private:
    MyAccount *myAcc;
    SocketMedia *player;
    SocketMedia *recorder;


public:
    MyCall(Account &acc, int call_id = PJSUA_INVALID_ID): Call(acc, call_id), player(NULL), recorder(NULL) {
        myAcc = (MyAccount *)&acc;
    }
    void setPlayer(SocketMedia *_player) {
        player = _player;
    }
    void setRecorder(SocketMedia *_recorder) {
        recorder = _recorder;
    }
    virtual void onCallState(OnCallStateParam &prm);
    virtual void onCallMediaState(OnCallMediaStateParam &prm);
};

class MyAccount : public Account
{
public:
    std::vector<Call *> calls;
    
public:
    MyAccount()
    {}

    ~MyAccount()
    {
        std::cout << "*** Account is being deleted: No of calls="
                  << calls.size() << std::endl;
    }
    
    void removeCall(Call *call)
    {
        for (std::vector<Call *>::iterator it = calls.begin();
             it != calls.end(); ++it)
        {
            if (*it == call) {
                calls.erase(it);
                break;
            }
        }
    }

    virtual void onRegState(OnRegStateParam &prm)
    {
	AccountInfo ai = getInfo();
	std::cout << (ai.regIsActive? "*** Register: code=" : "*** Unregister: code=")
		  << prm.code << std::endl;
    }
    
    virtual void onIncomingCall(OnIncomingCallParam &iprm)
    {
        Call *call = new MyCall(*this, iprm.callId);
        CallInfo ci = call->getInfo();
        CallOpParam prm;
        
        std::cout << "*** Incoming Call: " <<  ci.remoteUri << " ["
                  << ci.stateText << "]" << std::endl;
        
        calls.push_back(call);
        prm.statusCode = (pjsip_status_code)200;
        call->answer(prm);
    }
};

void MyCall::onCallState(OnCallStateParam &prm)
{
    PJ_UNUSED_ARG(prm);

    CallInfo ci = getInfo();
    std::cout << "*** Call: " <<  ci.remoteUri << " [" << ci.stateText
              << "]" << std::endl;
    
    if (ci.state == PJSIP_INV_STATE_DISCONNECTED) {
        myAcc->removeCall(this);
        /* Delete the call */
        delete this;
    }
}

void MyCall::onCallMediaState(OnCallMediaStateParam &prm) {
    CallInfo ci = getInfo();
    for(uint16_t i = 0;i < ci.media.size();i ++) {
        if(ci.media[i].type == PJMEDIA_TYPE_AUDIO && getMedia(i)) {
            AudioMedia *aud_med = (AudioMedia*) getMedia(i);
            aud_med->startTransmit(*recorder);
            player->startTransmit(*aud_med);
        }
    }
}

static void mainProg1(Endpoint &ep) throw(Error) // Make Call
{
    // Init library
    EpConfig ep_cfg;
    ep_cfg.logConfig.level = 4;
    ep.libInit( ep_cfg );

    // Transport
    TransportConfig tcfg;
    tcfg.port = 5074;
    ep.transportCreate(PJSIP_TRANSPORT_UDP, tcfg);

    // Start library
    ep.libStart();
    std::cout << "*** PJSUA2 STARTED ***" << std::endl;

    // Add account
    AccountConfig acc_cfg;
    acc_cfg.idUri = "sip:102@10.128.171.151";
    acc_cfg.regConfig.registrarUri = "sip:10.128.171.151";
    acc_cfg.sipConfig.authCreds.push_back( AuthCredInfo("10.128.171.151", "*",
                                                        "102", PJSIP_CRED_DATA_PLAIN_PASSWD, "102") ); // pjsip/include/pjsua2/siptypes.hpp
    MyAccount *acc(new MyAccount);
    acc->create(acc_cfg);
    std::cout<<"*** after creating ***"<<std::endl;
    // Make outgoing call

    SocketMedia player, recorder;
    player.createPlayer("Player", 500, 1000, 8);
    recorder.createRecorder("Recorder", 500, 1000, 8);
    MyCall *call = new MyCall(*acc);
    call->setPlayer(&player);
    call->setRecorder(&recorder);
    socket_thread::create_play_thread("10.128.171.151",9010);
    socket_thread::thread_add_play_port(player.getPlayPort());
    socket_thread::thread_add_record_port(recorder.getRecordPort(), "10.128.171.151", 9012);
    acc->calls.push_back((Call *)call);

    pj_thread_sleep(10000);
    ep.hangupAllCalls();
    socket_thread::exit_play_thread();
    pj_thread_sleep(4000);
    
    if(acc)
        delete acc;
    std::cout << "*** PJSUA2 SHUTTING DOWN ***" << std::endl;
}

static void mainProg3(Endpoint &ep) throw(Error) // Incoming Call
{
    const char *paths[] = { "../../../../tests/pjsua/wavs/input.16.wav",
			    "../../tests/pjsua/wavs/input.16.wav",
			    "input.16.wav"};
    unsigned i;
    const char *filename = NULL;

    // Init library
    EpConfig ep_cfg;
    ep.libInit( ep_cfg );

    for (i=0; i<PJ_ARRAY_SIZE(paths); ++i) {
       if (pj_file_exists(paths[i])) {
          filename = paths[i];
          break;
       }
    }

    if (!filename) {
	PJSUA2_RAISE_ERROR3(PJ_ENOTFOUND, "mainProg3()",
			   "Could not locate input.16.wav");
    }

    // Start library
    ep.libStart();
    std::cout << "*** PJSUA2 STARTED ***" << std::endl;

    /* Use Null Audio Device as main media clock. This is useful for improving
     * media clock (see also https://trac.pjsip.org/repos/wiki/FAQ#tx-timing)
     * especially when sound device clock is jittery.
     */
    ep.audDevManager().setNullDev();

    /* And install sound device using Extra Audio Device */
    ExtraAudioDevice auddev2(-1, -1);
    try {
	auddev2.open();
    } catch (...) {
	std::cout << "Extra sound device failed" << std::endl;
    }

    // Create player and recorder
    {
	AudioMediaPlayer amp;
	amp.createPlayer(filename);

	AudioMediaRecorder amr;
	amr.createRecorder("recorder_test_output.wav");

	amp.startTransmit(amr);
	if (auddev2.isOpened())
	    amp.startTransmit(auddev2);

	pj_thread_sleep(5000);
    }
}

int main()
{
    int ret = 0;
    Endpoint ep;

    try {
	ep.libCreate();

	mainProg1(ep);
	ret = PJ_SUCCESS;
    } catch (Error & err) {
	std::cout << "Exception: " << err.info() << std::endl;
	ret = 1;
    }

    try {
	ep.libDestroy();
    } catch(Error &err) {
	std::cout << "Exception: " << err.info() << std::endl;
	ret = 1;
    }

    if (ret == PJ_SUCCESS) {
	std::cout << "Success" << std::endl;
    } else {
	std::cout << "Error Found" << std::endl;
    }

    return ret;
}


