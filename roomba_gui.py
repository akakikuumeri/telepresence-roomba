import PySimpleGUI as sg
import warnings
import serial
import serial.tools.list_ports
import time

#sg.theme('DarkAmber')	# Add a touch of color
# All the stuff inside your window.
layout = [ 	[sg.Button('↑', key='-forward-', size=(4, 2))],
			[sg.Button('←', key='-left-', size=(2, 1)), sg.Button('→', key='-right-', size=(3, 1))],
			[sg.Button("↓", key='-back-', size=(4, 2))],
			[sg.Slider(range=(1, 100), orientation='v', size=(10, 20), default_value=50, key="camera")],
			[sg.Button('   1   ', key='-button1-')],
			]


# Create the Window
window = sg.Window('Robot Control', layout,  return_keyboard_events=True, finalize=True)

#bind Tkinter events to extend SimpleGUI functionality
#these follow Tkinter event string names, its very frustrating. But PysimpleGUI only has button release as an event
window['-left-'].bind('<Button-1>', '+PRESS+')
window['-left-'].bind('<Leave>', '+LEAVE+')
window['-right-'].bind('<Button-1>', '+PRESS+')
window['-right-'].bind('<Leave>', '+LEAVE+')
window['-forward-'].bind('<Button-1>', '+PRESS+')
window['-forward-'].bind('<Leave>', '+LEAVE+')
window['-back-'].bind('<Button-1>', '+PRESS+')
window['-back-'].bind('<Leave>', '+LEAVE+')
window['camera'].bind('<B1-Motion>', '+MOVE+')
window['-button1-'].bind('<Button-1>', '+PRESS+')
window['-button1-'].bind('<Leave>', '+LEAVE+')

window.bind('<Key>', '+KEY+')


#open serial
#ser = #serial.Serial('/dev/cu.usbserial-A603JLGD', 9600)


arduino_ports = [
    p.device
    for p in serial.tools.list_ports.comports()
    if 'UART' in p.description  # may need tweaking to match new arduinos
]
if not arduino_ports:
    raise IOError("No Arduino found")
if len(arduino_ports) > 1:
    warnings.warn('Multiple Arduinos found - using the first')

ser = serial.Serial(arduino_ports[0])

pan = 50 #default value for camera pan slider, to keep of changes

# Event Loop to process "events" and get the "values" of the inputs
while True:
	event, values = window.read()
	print(event, values)
	if event == sg.WIN_CLOSED:	# if user closes window or clicks cancel
		break
	
	if event == '-left-+PRESS+':
		ser.write(b"L0,5000 R255,5000\n") #go left for 5 seconds, if something happens the 5 s later it will stop. But the userr will have to press again after 5 s. Should be fine
	if event == '-left-' or event == '-left-+LEAVE+': #if button released, or mouse moves out of button,cancel the press
		ser.write(b"L127 R127\n")

	if event == '-right-+PRESS+':
		ser.write(b"L255,5000 R0,5000\n") #go right for 5 seconds, if something happens the 5 s later it will stop. But the userr will have to press again after 5 s. Should be fine
	if event == '-right-' or event == '-right-+LEAVE+': #if button released, or mouse moves out of button,cancel the press
		ser.write(b"L127 R127\n")

	if event == '-forward-+PRESS+':
		ser.write(b"L200,5000 R255,5000\n") #go forward for 5 seconds, if something happens the 5 s later it will stop. But the userr will have to press again after 5 s. Should be fine
	if event == '-forward-' or event == '-forward-+LEAVE+': #if button released, or mouse moves out of button,cancel the press
		ser.write(b"L127 R127\n")

	if event == '-back-+PRESS+':
		ser.write(b"L50,5000 R0,5000\n") #go backwards for 5 seconds, if something happens the 5 s later it will stop. But the userr will have to press again after 5 s. Should be fine
	if event == '-back-' or event == '-back-+LEAVE+': #if button released, or mouse moves out of button,cancel the press
		ser.write(b"L127 R127\n")

	if event == '-button1-+PRESS+':
		ser.write(b"B1\n") #go backwards for 5 seconds, if something happens the 5 s later it will stop. But the userr will have to press again after 5 s. Should be fine
	if event == '-button1-' or event == '-button1-+LEAVE+': #if button released, or mouse moves out of button,cancel the press
		ser.write(b"B0\n")

	#simple keyboard support too. Must tap on WASD to move
	if event == 'w':
		ser.write(b"L200,200 R255,200\n")
	if event == 'a':
		ser.write(b"L0,200 R255,200\n")
	if event == 's':
		ser.write(b"L0,200 R50,200\n")
	if event == 'd':
		ser.write(b"L255,200 R0,200\n")

	if event == "camera+MOVE+":
		val = values["camera"]
		if (val != pan): #only update if changed
			ser.write(b"C" + bytes(str(val), 'utf-8') + b"\n") #write C50 to serial in bytes
			print(b"C" + b"50" + b"\n")
			pan = val
			time.sleep(0.05) #serial gets congested because the MOVE event is spammed too much


window.close()