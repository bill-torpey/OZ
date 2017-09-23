/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2015 Frank Quinn (http://fquinner.github.io)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#ifndef MAMA_BRIDGE_ZMQ_TRANSPORT_H__
#define MAMA_BRIDGE_ZMQ_TRANSPORT_H__


/*=========================================================================
  =                             Includes                                  =
  =========================================================================*/

#include <stdlib.h>
#include <string.h>

#include <mama/mama.h>
#include <bridge.h>
#include "zmqdefs.h"

#if defined(__cplusplus)
extern "C" {
#endif

/**
 * This is a simple convenience function to return a zmqTransportBridge
 * pointer based on the provided mamaTransport.
 *
 * @param transport  The mamaTransport to extract the bridge transport from.
 *
 * @return zmqTransportBridge* associated with the mamaTransport.
 */
zmqTransportBridge*
zmqBridgeMamaTransportImpl_getTransportBridge(mamaTransport transport);

/**
 * This is purely a debug function to dump to screen a snapshot of the status
 * of the transport's message pool.
 *
 * @param impl       The zmq transport bridge referring to a message pool.
 */
void
zmqBridgeMamaTransportImpl_dumpMessagePool(zmqTransportBridge* impl);


mama_status zmqBridgeMamaTransportImpl_getInboxSubject(zmqTransportBridge* impl, const char** inboxSubject);


mama_status zmqBridgeMamaTransportImpl_registerInbox(zmqTransportBridge* impl, zmqInboxImpl* inbox);
mama_status zmqBridgeMamaTransportImpl_unregisterInbox(zmqTransportBridge* impl, zmqInboxImpl* inbox);

mama_status zmqBridgeMamaTransportImpl_sendCommand(zmqTransportBridge* impl, zmqControlMsg* msg, int msgSize);


#if defined(__cplusplus)
}
#endif

#endif /* MAMA_BRIDGE_ZMQ_TRANSPORT_H__*/
