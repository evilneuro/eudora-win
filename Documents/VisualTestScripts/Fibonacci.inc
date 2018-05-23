' File name: Fibonacci.inc
' Project:  WnObjPrg
Declare Function Fibonacci(fibnum&) as Long
Declare Function CalcFibonacciTime(MinutesTime&) as Long
Declare Function DoFibonacciClicks(fibnum&) as Long


'*************************************************************************
' Function Fibonacci
'
'      Jason Miller
'      jasmille@qualcomm.com
'
' Makes Fibonacci Calculations through recursion
'
' Parameters:
'
' fibnum&    Long variable to recurse
'
' Returns True/False
'
'*************************************************************************

Function Fibonacci(fibnum&) as Long
	Dim ReturnVal as Long

	If(fibnum < 2) Then
		Fibonacci = 1
		Exit Function
	EndIf

	Fibonacci = Fibonacci(fibnum - 1) + Fibonacci(fibnum - 2) 

End Function

'*************************************************************************
' Function CalcFibonacciTime
'
'      Jason Miller
'      jasmille@qualcomm.com
'
' Figures out which Fibonacci Number to use based on Time given in minutes
'
' Parameters:
'
' MinutesTime&    Time in minutes to run
'
' Returns True/False
'
'*************************************************************************
Function CalcFibonacciTime(MinutesTime&) as Long
	Dim fibnum as Long
	Dim tally as Long
	Dim SecondsTime as Long
	Dim I as Integer

	tally = 0
	I = 0
	SecondsTime = MinutesTime * 60

	While tally < SecondsTime
		tally = tally + Fibonacci(I)
		I = I + 1
	WEnd
	I = I - 1

	CalcFibonacciTime = I
	
End Function
'*************************************************************************
' Function CalcFibonacciTime
'
'      Jason Miller
'      jasmille@qualcomm.com
'
' Figures out which Fibonacci Number to use based on Time given in minutes
'
' Parameters:
'
' fibnum&    Fibonacci Number we will use to do clicks
'
' Returns True/False
'
'*************************************************************************
Function DoFibonacciClicks(fibnum&) as Long
	Dim I as Long

	For I = 0 To fibnum
		Play "{Click 744, 573, Left}"
		Sleep Fibonacci(I)
	Next I

End Function

