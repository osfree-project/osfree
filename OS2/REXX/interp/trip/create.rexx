/* 

*/
out1 = "small.txt"
out2 = "big.txt"

call stream out1, "C", "Open Write Replace"
call stream out2, "C", "Open Write Replace"

crlf = d2c(13) || d2c(10)
same = copies("X", 90)

X = 100
do i = 1 to X
    text = left(i, 8) || same || crlf
    call charout out1, text
    call charout out2, text
    end
	
do i = X + 1 to 10000
    text = left(i, 8) || same || crlf
    call charout out2, text
    end
	
	
	
call stream out1, "C", "Close"
call stream out2, "C", "Close"
