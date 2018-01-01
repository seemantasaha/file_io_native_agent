#!/bin/bash

echo "Building native agent..."
g++ -Ofast -std=c++11 -fPIC -Wno-write-strings \
  -I/usr/lib/jvm/java-8-oracle/include \
  -I/usr/lib/jvm/java-8-oracle/include/linux \
  file-io-event.h file-io-event.cpp native-agent.cpp -shared -olibnativeagent.so

echo "Running native agent..."
java -agentpath:$(realpath libnativeagent.so) -cp . WriteToFileExample
