// minimal wrapper for OpenMAMA API

#include <string>
using namespace std;

#include <mama/mama.h>

#include "../src/util.h"

#include "ozimpl.h"

namespace oz {

///////////////////////////////////////////////////////////////////////
// connection
oz::connection* oz::connection::create(string mw, string payload, string name)
{
   oz::connection* pConn = new oz::connection(mw, payload, name);
   return pConn;
}

oz::connection::connection(string mw, string payload, string name)
   : status_(MAMA_STATUS_INVALID_ARG), bridge_(nullptr), queue_(nullptr), transport_(nullptr), payloadBridge_(nullptr)
{
   mw_ = mw;
   payload_ = payload;
   name_ = name;
}

mama_status oz::connection::start(void)
{
   CALL_MAMA_FUNC(status_ = mama_loadBridge(&bridge_, mw_.c_str()));
   CALL_MAMA_FUNC(status_ = mama_loadPayloadBridge(&payloadBridge_, payload_.c_str()));
   CALL_MAMA_FUNC(status_ = mama_open());
   CALL_MAMA_FUNC(status_ = mama_getDefaultEventQueue(bridge_, &queue_));
   CALL_MAMA_FUNC(status_ = mamaTransport_allocate(&transport_));
   CALL_MAMA_FUNC(status_ = mamaTransport_create(transport_, name_.c_str(), bridge_));
   CALL_MAMA_FUNC(status_ = mama_startBackgroundEx(bridge_, onStop, this));
   return MAMA_STATUS_OK;
}

mama_status oz::connection::stop(void)
{
   CALL_MAMA_FUNC(status_ = mama_stop(bridge_));
   return MAMA_STATUS_OK;
}

mama_status oz::connection::destroy(void)
{
   CALL_MAMA_FUNC(status_ = mamaTransport_destroy(transport_));
   CALL_MAMA_FUNC(status_ = mama_close());
   delete this;
   return MAMA_STATUS_OK;
}

void MAMACALLTYPE oz::connection::onStop(mama_status status, mamaBridge bridge, void* closure)
{
   connection* pThis = static_cast<connection*>(closure);
}


///////////////////////////////////////////////////////////////////////
// session
oz::session* oz::session::create(oz::connection* pConn)
{
   oz::session* pSession = new oz::session(pConn);
   return pSession;
}

oz::session::session(oz::connection* pConn)
   : pConn_(pConn)
{
}

mama_status oz::session::start(void)
{
   CALL_MAMA_FUNC(status_ = mamaQueue_create(&queue_, pConn_->bridge()));
   CALL_MAMA_FUNC(status_ = mamaDispatcher_create(&dispatcher_, queue_));
   return MAMA_STATUS_OK;
}

mama_status oz::session::stop(void)
{
   CALL_MAMA_FUNC(status_ = mamaDispatcher_destroy(dispatcher_));
   return MAMA_STATUS_OK;
}

mama_status oz::session::destroy(void)
{
   CALL_MAMA_FUNC(status_ = mamaQueue_destroy(queue_));
   delete this;
   return MAMA_STATUS_OK;
}


///////////////////////////////////////////////////////////////////////
// subscriber
subscriber* subscriber::create(session* pSession, std::string topic)
{
   return new subscriber(pSession, topic);
}

subscriber::subscriber(session* pSession, std::string topic)
   : pSession_(pSession), sub_(nullptr), topic_(topic)
{
}

mama_status subscriber::destroy()
{
   CALL_MAMA_FUNC(mamaSubscription_destroyEx(sub_));
   // Note: delete is done in destroyCB
   return MAMA_STATUS_OK;
}

subscriber::~subscriber() {}

mama_status subscriber::subscribe()
{
   mamaMsgCallbacks cb;
   memset(&cb, 0, sizeof(cb));
   cb.onCreate       = createCB;
   cb.onError        = errorCB;
   cb.onMsg          = msgCB;
   cb.onQuality      = nullptr;
   cb.onGap          = nullptr;
   cb.onRecapRequest = nullptr;
   cb.onDestroy      = destroyCB;

   CALL_MAMA_FUNC(mamaSubscription_allocate(&sub_));
   CALL_MAMA_FUNC(mamaSubscription_createBasic(sub_, pSession_->connection()->transport(), pSession_->queue(), &cb, topic_.c_str(), this));
   return MAMA_STATUS_OK;
}

void MAMACALLTYPE subscriber::createCB(mamaSubscription subscription, void* closure)
{
   subscriber* cb = dynamic_cast<subscriber*>(static_cast<subscriber*>(closure));
   if (cb) {
      cb->onCreate();
   }
}

void MAMACALLTYPE subscriber::errorCB(mamaSubscription subscription, mama_status status, void* platformError, const char* subject, void* closure)
{
   subscriber* cb = dynamic_cast<subscriber*>(static_cast<subscriber*>(closure));
   if (cb) {
      cb->onError(status, platformError, subject);
   }
}

void MAMACALLTYPE subscriber::msgCB(mamaSubscription subscription, mamaMsg msg, void* closure, void* itemClosure)
{
   subscriber* cb = dynamic_cast<subscriber*>(static_cast<subscriber*>(closure));
   if (cb) {
      cb->onMsg(msg, itemClosure);
   }
}

void MAMACALLTYPE subscriber::destroyCB(mamaSubscription subscription, void* closure)
{
   subscriber* cb = dynamic_cast<subscriber*>(static_cast<subscriber*>(closure));
   if (cb) {
      delete cb;
   }
}

// no-op definitions
void MAMACALLTYPE subscriber::onCreate(void) {}
void MAMACALLTYPE subscriber::onError(mama_status status, void* platformError, const char* subject) {}
void MAMACALLTYPE subscriber::onMsg(mamaMsg msg, void* itemClosure) {}


///////////////////////////////////////////////////////////////////////
// publisher
publisher* publisher::create(connection* pConnection, std::string topic)
{
   return new publisher(pConnection, topic);
}

publisher::publisher(connection* pConnection, std::string topic)
   : pConn_(pConnection), pub_(nullptr), topic_(topic)
{
}

mama_status publisher::destroy(void)
{
   delete this;
   return MAMA_STATUS_OK;
}

publisher::~publisher() {}

mama_status publisher::publish(mamaMsg msg)
{
   if (pub_ == nullptr) {
      CALL_MAMA_FUNC(mamaPublisher_create(&pub_, pConn_->transport(), topic_.c_str(), NULL, NULL));
   }

   return mamaPublisher_send(pub_, msg);
}


///////////////////////////////////////////////////////////////////////
// signal handling
void ignoreSigHandler(int sig) {}
void hangout(void)
{
   signal(SIGINT, ignoreSigHandler);
   pause();
}


///////////////////////////////////////////////////////////////////////
// inbox
oz::inbox* inbox::create(session* pSession, std::string topic)
{
   return new inbox(pSession, topic);
}

inbox::inbox(session* pSession, std::string topic)
   : pSession_(pSession), inbox_(nullptr), pub_(nullptr), topic_(topic)
{
}

mama_status inbox::destroy()
{
   CALL_MAMA_FUNC(mamaInbox_destroy(inbox_));
   // Note: delete is done in destroyCB
   return MAMA_STATUS_OK;
}

inbox::~inbox() {}

mama_status inbox::sendRequest(mamaMsg msg)
{
   if (inbox_ == nullptr) {
      CALL_MAMA_FUNC(mamaInbox_create2(&inbox_, pSession_->connection()->transport(), pSession_->queue(), msgCB, errorCB, destroyCB, this));
   }
   if (pub_ == nullptr) {
      CALL_MAMA_FUNC(mamaPublisher_create(&pub_, pSession_->connection()->transport(), topic_.c_str(), NULL, NULL));
   }

   if (inbox_ && pub_) {
      return mamaPublisher_sendFromInbox(pub_, inbox_, msg);
   }

   return MAMA_STATUS_INVALID_ARG;
}


void MAMACALLTYPE inbox::errorCB(mama_status status, void* closure)
{
   inbox* cb = dynamic_cast<inbox*>(static_cast<inbox*>(closure));
   if (cb) {
      cb->onError(status);
   }
}

void MAMACALLTYPE inbox::msgCB(mamaMsg msg, void* closure)
{
   inbox* cb = dynamic_cast<inbox*>(static_cast<inbox*>(closure));
   if (cb) {
      cb->onReply(msg);
   }
}

void MAMACALLTYPE inbox::destroyCB(mamaInbox inbox, void* closure)
{
   oz::inbox* cb = dynamic_cast<oz::inbox*>(static_cast<oz::inbox*>(closure));
   if (cb) {
      delete cb;
   }
}

// no-op definitions
void MAMACALLTYPE inbox::onError(mama_status status) {}
void MAMACALLTYPE inbox::onReply(mamaMsg msg) {}

}

