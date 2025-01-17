/**
Software License Agreement (BSD)

\file      serial.h
\authors   Jacob Perron <jperron@sfu.ca>
\copyright Copyright (c) 2015, Autonomy Lab (Simon Fraser University), All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
 * Redistributions of source code must retain the above copyright notice,
   this list of conditions and the following disclaimer.
 * Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.
 * Neither the name of Autonomy Lab nor the names of its contributors may
   be used to endorse or promote products derived from this software without
   specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
POSSIBILITY OF SUCH DAMAGE.
*/

// Based on example from:
//   https://github.com/labust/labust-ros-pkg/wiki/Create-a-Serial-Port-application

#ifndef CREATE_SERIAL_H
#define CREATE_SERIAL_H

#include <condition_variable>
#include <functional>
#include <memory>
#include <mutex>
#include <thread>

#include <boost/asio.hpp>

#include "create/data.h"
#include "create/types.h"
#include "create/util.h"

namespace create {
  class Serial : public std::enable_shared_from_this<Serial> {

    protected:
      boost::asio::io_service io;
      boost::asio::signal_set signals;
      boost::asio::serial_port port;

    private:
      std::thread ioThread;
      std::condition_variable dataReadyCond;
      std::mutex dataReadyMut;
      bool dataReady;
      bool isReading;
      bool firstRead;
      uint8_t byteRead;

      // Callback executed when data arrives from Create
      void onData(const boost::system::error_code& e, const std::size_t& size);
      // Callback to execute once data arrives
      std::function<void()> callback;
      // Start and stop reading data from Create
      bool startReading();
      void stopReading();

    protected:
      std::shared_ptr<Data> data;
      // These are for possible diagnostics
      uint64_t corruptPackets;
      uint64_t totalPackets;

      virtual bool startSensorStream() = 0;
      virtual void processByte(uint8_t byteRead) = 0;

      void signalHandler(const boost::system::error_code& error, int signal_number);
      // Notifies main thread that data is fresh and makes the user callback
      void notifyDataReady();

    public:
      Serial(std::shared_ptr<Data> data, bool install_signal_handler);
      ~Serial();
      bool connect(const std::string& port, const int& baud = 115200, std::function<void()> cb = 0);
      void disconnect();
      inline bool connected() const { return port.is_open(); };
      bool send(const uint8_t* bytes, const uint32_t numBytes);
      bool sendOpcode(const Opcode& code);
      uint64_t getNumCorruptPackets() const;
      uint64_t getTotalPackets() const;
      void setRTS(bool enabled);
      void setDTR(bool enabled);
  };
}  // namespace create

#endif // CREATE_SERIAL_H
