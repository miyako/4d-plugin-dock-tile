

Case of 
	: (Form event code:C388=On Load:K2:1)
		
		Form:C1466.progress:=0
		Form:C1466.colors:=New collection:C1472(\
			New object:C1471("red"; 1; "green"; 1; "blue"; 1); \
			New object:C1471("red"; 1; "green"; 0.3; "blue"; 0.3); \
			New object:C1471("red"; 0.3; "green"; 1; "blue"; 0.3); \
			New object:C1471("red"; 0.3; "green"; 0.3; "blue"; 1)\
			)
		Form:C1466.colorIndex:=0
		Form:C1466.options:=New object:C1471("style"; 0; "radius"; 40; "color"; Form:C1466.color[Form:C1466.colorIndex])
		
	: (Form event code:C388=On Clicked:K2:4)
		
		Form:C1466.activated:=Not:C34(Bool:C1537(Form:C1466.activated))
		If (Form:C1466.activated)
			SET TIMER:C645(1)
		Else 
			SET TIMER:C645(0)
			Form:C1466.progress:=0
			DOCK SET PROGRESS(Form:C1466.progress)
		End if 
		
	: (Form event code:C388=On Timer:K2:25)
		
		Form:C1466.progress:=Form:C1466.progress+0.01
		If (Form:C1466.progress>1)  // loop 0 to 1
			Form:C1466.progress:=Form:C1466.progress-1
			Form:C1466.options.style:=Form:C1466.options.style+1
			If (Form:C1466.options.style>2)  // loop style from 0 to 2
				Form:C1466.options.style:=0
				Form:C1466.colorIndex:=Form:C1466.colorIndex+1
				If (Form:C1466.colorIndex>=Form:C1466.colors.length)
					Form:C1466.colorIndex:=0
				End if 
				Form:C1466.options.color:=Form:C1466.colors[Form:C1466.colorIndex]
			End if 
		End if 
		
		DOCK SET PROGRESS(Form:C1466.progress; Form:C1466.options)
		
	Else 
		
End case 