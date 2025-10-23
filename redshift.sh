pgrep redshift  | xargs -n1 kill -9
redshift-gtk -x
redshift-gtk & disown

# https://github.com/jonls/redshift
#
# Hi
#
# I had this problem because my proxy doesn't let me connect.
#
# My Solution came from http://jonls.dk/redshift/ like jonls said.
#
# In my case: (Colombia)
#
# Open a terminal and write this
# redshift -l 4.65:-74.06 -t 5700:3600 -g 0.8 -m randr -v &
#
# Wait a couple of seconds and push ctr + c. And That's it.
#
# In your case
#
#Open a terminal ant write this
#redshift -l [your latitude]:[your longitude] -t 5700:3600 -g 0.8 -m randr -v &
#Wait a couple of seconds and push ctr + c. And That's it.
#

redshift -l 4.65:-74.06 -t 5700:3600 -g 0.8 -m randr -v &
