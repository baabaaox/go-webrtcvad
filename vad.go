package webrtcvad

// #cgo linux CPPFLAGS: -DWEBRTC_POSIX
// #cgo darwin CPPFLAGS: -DWEBRTC_POSIX
// #cgo windows CPPFLAGS: -DWEBRTC_WIN
// #cgo CPPFLAGS: -I${SRCDIR}/webrtc_lkgr
// #cgo CXXFLAGS: -std=c++11
// #include "webrtc_lkgr/common_audio/vad/webrtc_vad.c"
import "C"
import (
	"errors"
	"unsafe"
)

// Create Creates an instance to the VAD structure.
func Create() *C.struct_WebRtcVadInst {
	return C.WebRtcVad_Create()
}

// Free Frees the dynamic memory of a specified VAD instance.
func Free(vadInst *C.struct_WebRtcVadInst) {
	C.WebRtcVad_Free(vadInst)
}

// Init Initializes a VAD instance.
func Init(vadInst *C.struct_WebRtcVadInst) (err error) {
	result := C.WebRtcVad_Init(vadInst)
	if result == -1 {
		err = errors.New("null pointer or Default mode could not be set")
	}
	return
}

// SetMode Sets the VAD operating mode.
func SetMode(vadInst *C.struct_WebRtcVadInst, mode int) (err error) {
	result := C.WebRtcVad_set_mode(vadInst, C.int(mode))
	if result == -1 {
		err = errors.New("mode could not be set or the VAD instance has not been initialized")
	}
	return
}

// Process Sets the VAD operating mode.
func Process(vadInst *C.struct_WebRtcVadInst, fs int, audioFrame []byte, frameLength int) (active bool, err error) {
	result := C.WebRtcVad_Process(vadInst, C.int(fs), (*C.short)(unsafe.Pointer(&audioFrame[0])), C.size_t(frameLength))
	if result == 1 {
		active = true
	} else if result == 0 {
		active = false
	} else {
		err = errors.New("process fail")
	}
	return
}

// ValidRateAndFrameLength Sets the VAD operating mode.
func ValidRateAndFrameLength(rate int, frameLength int) (valid bool) {
	result := C.WebRtcVad_ValidRateAndFrameLength(C.int(rate), C.size_t(frameLength))
	if result == 0 {
		valid = true
	} else {
		valid = false
	}
	return
}
