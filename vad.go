package webrtcvad

// #cgo CPPFLAGS: -I${SRCDIR}/webrtc_lkgr
// #include "common_audio/signal_processing/resample_by_2_internal.c"
// #include "common_audio/signal_processing/spl.c"
// #include "common_audio/vad/vad_filterbank.c"
// #include "common_audio/vad/vad_core.c"
// #include "common_audio/vad/vad_gmm.c"
// #include "common_audio/vad/vad_sp.c"
// #include "common_audio/vad/webrtc_vad.c"
import "C"
import (
	"errors"
	"unsafe"
)

type VadInst *C.struct_WebRtcVadInst

// Create Creates an instance to the VAD structure.
func Create() VadInst {
	return VadInst(C.WebRtcVad_Create())
}

// Free Frees the dynamic memory of a specified VAD instance.
func Free(vadInst VadInst) {
	C.WebRtcVad_Free(vadInst)
}

// Init Initializes a VAD instance.
func Init(vadInst VadInst) (err error) {
	result := C.WebRtcVad_Init(vadInst)
	if result == -1 {
		err = errors.New("null pointer or Default mode could not be set")
	}
	return
}

// SetMode Sets the VAD operating mode.
func SetMode(vadInst VadInst, mode int) (err error) {
	result := C.WebRtcVad_set_mode(vadInst, C.int(mode))
	if result == -1 {
		err = errors.New("mode could not be set or the VAD instance has not been initialized")
	}
	return
}

// Process Sets the VAD operating mode.
func Process(vadInst VadInst, fs int, audioFrame []byte, frameLength int) (active bool, err error) {
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
