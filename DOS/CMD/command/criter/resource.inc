include "resource.id"
resIdBlock macro major, minor
	dd ???ende - ???start			;; Length of this resource
	dw major				;; major resource ID
	dw minor				;; minor resource ID
	db 'FREECOM '				;; cookie
endm
