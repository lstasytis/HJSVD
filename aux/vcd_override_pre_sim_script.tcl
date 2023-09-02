puts "Started script to override default waveform generation with a vcd dump"
open_vcd
log_vcd *
log_vcd [ get_objects *]
run 5000
flush_vcd
close_vcd
exec cp dump.vcd ../../../../../../../../
quit

