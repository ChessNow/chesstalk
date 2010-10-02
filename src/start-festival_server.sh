#!/bin/bash

exec festival --server

# can also start with jack output
# usually by having a .asoundrc in your home directory so that the alsa aplay command will go out through jack and your jackd daemon.
# can the alsaplayer command be used instead to read .au or some other audio format directly for jack_client output?
