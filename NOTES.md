# Unsupported PCRE constructs

* `\ddd` octal numbers without the leading zero (`\o{ddd}` and `\0dd` work)
* `\Q...\E` quoting
* `\C` data unit
* `\h` `\H` horizontal character classes
* `\v` `\V` vertical character classes
* `\p{xx}` `\P{xx}` character properties
* `\X` matches the atomic `\P{M}\p{M}*` approximation, not full UAX #29 clusters
* boundaries other than `^$`
* atomic groups
* options/modes
* recursion (subroutine calls are supported, but must not be recursive)


# Other unsupported "things"
* `[[.hyphen.]]` named characters
* `[[=M=]]` whatever this is

