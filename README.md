ANTONI SOLARSKI 148270
SI3 - Artificial Intelligence

# "The producer-consumer problem revisited"

# General info
This program realises the second project from the oscp coursebook.
There is a buffer of fixed size. The producer produces and puts items on this buffer (**add** operation). The consumer takes the item (**take** operation) and consumes it. The buffer works like stack (the **LIFO** queue). Producer and consumer functions runs in while loop until number of produced (or consumed) items hits the limit. As there are several producers and consumers, there are some synchronization methods addapted. Firstly before any add or take operation the buffer is locked (use of mutex). Secondly, when one of producers or consumers has to increment some shared variables there are locked with another mutexes. Also we have to prevent deadlock. Using semaphores - producer waits when the buffer is full and consumer waits when the buffer is emtpy. At the end it's worth to mention that in such solution no items are wasted by producers (i.e. produced and not added to buffer) because produers stop producing items when they hit the limit and once the item is produced the producer waits for free space in buffer.

# Running the program
To compile the program just use "**make**" command without any additional parameters.

If program is run **without** parameters allows to set buffer size, number of producers and consumers and time of production (consumption) for each producer (consumer) respectively. Also it allows to set the number of items to be produced and consumed.
One is asked to enter all this parameters from the standard input.
If it is run with any parameter the above values are set to **default** ones (defined in source code).

While running the program prints when any producer (consumer) start and ends producing (consuming) item.
Also after any **add** or **take** operation it prints the curent buffer state and number of items produced and consumed so far.
At the end it prints the buffer (should be *empty*, -1 means that no item is present at certain position) and total numbers of produced and consumed items (should be equal and equal items limit). 
