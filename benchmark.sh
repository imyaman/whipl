#!/bin/bash

echo "whipl vs curl benchmark"
echo "======================="
echo

URL="http://example.com"

echo "Testing whipl (5 requests):"
for i in {1..5}; do
    /usr/bin/time -f "  Request $i: %E elapsed" ./bin/whipl $URL > /dev/null 2>&1
done

echo
echo "Testing curl (5 requests):"
for i in {1..5}; do
    /usr/bin/time -f "  Request $i: %E elapsed" curl -s $URL > /dev/null 2>&1
done
