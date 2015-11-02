# Write a regular expression matching strings of all a or b such as aababa or bbaa

/[a,b]+/

# Write a regular expression matching strings of consecutive a and b

/(ab)+a?/

# Write a regular expression matching pit, pot, respite but not peat, spit, or part
/^(p.t|.+p.t.+)/
