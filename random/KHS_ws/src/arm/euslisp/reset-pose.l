#!/usr/bin/env roseus

(load "package://arm/euslisp/arm-interface.l")
(arm-init)

(send *ri* :servo-on)
(unix::sleep 3)
(send *ri* :angle-vector (send *arm* :reset-pose) 3000)
(send *ri* :wait-interpolation)
