# go-webrtcvad

[![Build Status]( https://dev.azure.com/baabaaox/go-webrtcvad/_apis/build/status/7?branchName=master)](https://dev.azure.com/baabaaox/go-webrtcvad/_build/latest?definitionId=7&branchName=master)

This project rewrite from [maxhawkins/go-webrtcvad](https://github.com/maxhawkins/go-webrtcvad). The WebRTC source code download form [WebRTC lkgr](https://webrtc.googlesource.com/src/+/refs/heads/lkgr). The Abseil Common Libraries (C++) source code download from [abseil/abseil-cpp](https://github.com/abseil/abseil-cpp). Need C++ compiler with support for C++11.

## Installation

```shell
go get github.com/baabaaox/go-webrtcvad
```

## Example

```go
package main

import (
    "io"
    "log"
    "os"
    "github.com/baabaaox/go-webrtcvad"
)

const (
    // VadMode vad mode
    VadMode = 0
    // SimpleRate simple rate
    SimpleRate = 16000
    // BitDepth bit depth
    BitDepth = 16
    // FrameDuration frame duration
    FrameDuration = 20
)

var (
    audioFile   *os.File
    frameIndex  = 0
    frameSize   = SimpleRate / 1000 * FrameDuration
    frameBuffer = make([]byte, SimpleRate/1000*FrameDuration*BitDepth/8)
    frameActive = false
)

func main() {
    // ffmpeg -y -i test.mp4 -acodec pcm_s16le -f s16le -ac 1 -ar 16000 test.pcm
    audioFile, err := os.Open("test.pcm")
    if err != nil {
        log.Fatal(err)
    }
    defer audioFile.Close()
    vadInst := webrtcvad.Create()
    defer webrtcvad.Free(vadInst)
    webrtcvad.Init(vadInst)
    if err != nil {
        log.Fatal(err)
    }
    err = webrtcvad.SetMode(vadInst, VadMode)
    if err != nil {
        log.Fatal(err)
    }
    for {
        _, err = audioFile.Read(frameBuffer)
        if err == io.EOF {
            break
        }
        if err != nil {
            return
        }
        frameActive, err = webrtcvad.Process(vadInst, SimpleRate, frameBuffer, frameSize)
        if err != nil {
            log.Fatal(err)
        }
        log.Printf("Frame: %v, Active: %v", frameIndex, frameActive)
        frameIndex++
    }
}
```
