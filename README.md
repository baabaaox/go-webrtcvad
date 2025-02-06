# go-webrtcvad

[![Go Tests](https://github.com/baabaaox/go-webrtcvad/actions/workflows/go-tests.yml/badge.svg)](https://github.com/baabaaox/go-webrtcvad/actions/workflows/go-tests.yml)

I have ported VAD from latest WebRTC lkgr at 20250207.

This project rewrite from [maxhawkins/go-webrtcvad](https://github.com/maxhawkins/go-webrtcvad).

The WebRTC source code download form [WebRTC lkgr commit [8e55dca89f4e39241f9e3ecd25ab0ebbf5d1ab37]](https://webrtc.googlesource.com/src/+/8e55dca89f4e39241f9e3ecd25ab0ebbf5d1ab37).

## Installation

```shell
go get github.com/baabaaox/go-webrtcvad
```

## Example

```go
package main

import (
	"bytes"
	"encoding/binary"
	"io"
	"log"
	"os"
	"github.com/baabaaox/go-webrtcvad"
)

const (
	// VadMode vad mode
	VadMode = 0
	// SampleRate sample rate
	SampleRate = 16000
	// BitDepth bit depth
	BitDepth = 16
	// FrameDuration frame duration
	FrameDuration = 20
)

var (
	frameIndex  = 0
	frameSize   = SampleRate / 1000 * FrameDuration
	frameBuffer = make([]byte, SampleRate/1000*FrameDuration*BitDepth/8)
	frameActive = false
)

func pcm2wav(pcmBuffer []byte, file string) (err error) {
	wavBuffer := &bytes.Buffer{}
	// RIFF chunk
	wavBuffer.Write([]byte("RIFF"))
	binary.Write(wavBuffer, binary.LittleEndian, uint32(36+len(pcmBuffer)))
	wavBuffer.Write([]byte("WAVE"))
	// fmt chunk
	wavBuffer.Write([]byte("fmt "))
	binary.Write(wavBuffer, binary.LittleEndian, uint32(16))
	binary.Write(wavBuffer, binary.LittleEndian, uint16(1))
	binary.Write(wavBuffer, binary.LittleEndian, uint16(1))
	binary.Write(wavBuffer, binary.LittleEndian, uint32(16000))
	binary.Write(wavBuffer, binary.LittleEndian, uint32(1*16000*16/8))
	binary.Write(wavBuffer, binary.LittleEndian, uint16(1*16/8))
	binary.Write(wavBuffer, binary.LittleEndian, uint16(16))
	// data chunk
	wavBuffer.Write([]byte("data"))
	binary.Write(wavBuffer, binary.LittleEndian, uint32(len(pcmBuffer)))
	wavBuffer.Write(pcmBuffer)
	err = os.WriteFile(file, wavBuffer.Bytes(), 0777)
	return
}

func main() {
	// ffmpeg -y -i test.mp4 -acodec pcm_s16le -f s16le -ac 1 -ar 16000 test.pcm
	// ffmpeg -y -i test.mp3 -acodec pcm_s16le -f s16le -ac 1 -ar 16000 test.pcm
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
	chunkBuffer := &bytes.Buffer{}
	for {
		_, err = audioFile.Read(frameBuffer)
		if err == io.EOF {
			break
		}
		if err != nil {
			return
		}
		frameActive, err = webrtcvad.Process(vadInst, SampleRate, frameBuffer, frameSize)
		if err != nil {
			log.Fatal(err)
		}
		if frameActive {
			chunkBuffer.Write(frameBuffer)
		}
		log.Printf("Frame: %v, Active: %v", frameIndex, frameActive)
		frameIndex++
	}
	pcm2wav(chunkBuffer.Bytes(), "test.wav")
}
```
