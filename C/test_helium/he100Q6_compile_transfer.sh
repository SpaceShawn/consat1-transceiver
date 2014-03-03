#!/bin/bash
make buildQ6
scp -P2222 bin/test_he100Q6.x root@lt-stone.encs.concordia.ca:/home/CODE/
