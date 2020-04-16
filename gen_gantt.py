p = [(6, 3), (1, 1), (2, 5), (3, 4), (5, 2)]

from tkinter import *
master = Tk()

canvas = Canvas(master, width=1000, height=1000)
canvas.pack()
elLabel = canvas.create_text(500, 100, text="P1", fill="blue")
master.mainloop()
