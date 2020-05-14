package webrtcvad

import (
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
