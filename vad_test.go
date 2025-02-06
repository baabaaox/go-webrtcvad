package webrtcvad

import (
	"io"
	"os"
	"testing"
)

func TestSetMode(t *testing.T) {
	vadInst := Create()
	defer Free(vadInst)
	Init(vadInst)
	for i := 0; i < 4; i++ {
		err := SetMode(vadInst, i)
		if err != nil {
			t.Errorf("mode = %v, expected = %v", i, err)
		}
	}
}

func TestProcess(t *testing.T) {
	vadInst := Create()
	defer Free(vadInst)
	Init(vadInst)
	rates := []int{8000, 16000, 32000, 48000}

	for _, rate := range rates {
		length := rate / 1000 * 10
		frame := make([]byte, length)
		active, _ := Process(vadInst, rate, frame, length)
		if active {
			t.Errorf("rate = %v, length = %v, expected = process fail", rate, length)
		}
	}
}

func TestValidRateAndFrameLength(t *testing.T) {
	rates := []int{8000, 16000, 32000, 48000}
	for _, rate := range rates {
		for i := 10; i < 40; i = i + 10 {
			length := rate / 1000 * i
			if !ValidRateAndFrameLength(rate, length) {
				t.Errorf("rate = %v, length = %v, expected = validate fail", rate, length)
			}
		}
	}
}

func TestPCM(t *testing.T) {
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

	audioFile, err := os.Open("./test/test.pcm")
	if err != nil {
		t.Errorf("expected = %v", err)
	}
	defer audioFile.Close()
	vadInst := Create()
	defer Free(vadInst)
	Init(vadInst)
	if err != nil {
		t.Errorf("expected = %v", err)
	}
	err = SetMode(vadInst, VadMode)
	if err != nil {
		t.Errorf("expected = %v", err)
	}
	for {
		_, err = audioFile.Read(frameBuffer)
		if err == io.EOF {
			break
		}
		if err != nil {
			t.Errorf("expected = %v", err)
		}
		frameActive, err = Process(vadInst, SampleRate, frameBuffer, frameSize)
		if err != nil {
			t.Errorf("expected = %v", err)
		}
		t.Logf("Frame: %v, Active: %v", frameIndex, frameActive)
		frameIndex++
	}
}
