import curses

# initialize curses
stdscr = curses.initscr()

# disable keyboard echo
curses.noecho()

# react to keys instantly
curses.cbreak()

# enable keypad mode 
stdscr.keypad(1)

# setup rectangular area of the screen (y,x)
begin_x = 20; begin_y = 7
height = 5; width = 40
win = curses.newwin(height, width, begin_y, begin_x)
stdscr.refresh()

'''
# pad
pad = curses.newpad(100, 100)
#  These loops fill the pad with letters; this is
# explained in the next section
for y in range(0, 100):
  for x in range(0, 100):
    try:
        pad.addch(y,x, ord('a') + (x*x+y*y) % 26)
    except curses.error:
        pass

#  Displays a section of the pad in the middle of the screen
pad.refresh(0,0, 5,5, 20,75)
'''

# input text widget
while 1:
  c = stdscr.getch()
  if c == ord('p'):
      PrintDocument()
  elif c == ord('q'):
      break  # Exit the while()
  elif c == curses.KEY_HOME:
      x = y = 0

# terminate application
curses.nocbreak(); stdscr.keypad(0); curses.echo()

# reverse the curses settings placed on the terminal

