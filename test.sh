#!/bin/bash
set -e

echo "whipl Test Suite"
echo "==============="
echo

# Test 1: Simple GET
echo "✓ Testing GET request..."
./bin/whipl http://example.com > /dev/null

# Test 2: HTTPS GET
echo "✓ Testing HTTPS request..."
./bin/whipl https://example.com > /dev/null

# Test 3: POST with data
echo "✓ Testing POST with data..."
./bin/whipl -X POST -d 'test=data' http://httpbin.org/post > /dev/null

# Test 4: HTTPS POST
echo "✓ Testing HTTPS POST..."
./bin/whipl -X POST -d 'secure=data' https://httpbin.org/post > /dev/null

# Test 5: Custom headers
echo "✓ Testing custom headers..."
./bin/whipl -H 'User-Agent: whipl-test' http://httpbin.org/headers > /dev/null

# Test 6: Basic auth
echo "✓ Testing basic authentication..."
./bin/whipl -u 'user:pass' http://httpbin.org/basic-auth/user/pass > /dev/null

# Test 7: HTTPS with basic auth
echo "✓ Testing HTTPS with basic auth..."
./bin/whipl -u 'user:pass' https://httpbin.org/basic-auth/user/pass > /dev/null

# Test 8: PUT request
echo "✓ Testing PUT request..."
./bin/whipl -X PUT -d '{"key":"value"}' http://httpbin.org/put > /dev/null

# Test 9: DELETE request
echo "✓ Testing DELETE request..."
./bin/whipl -X DELETE http://httpbin.org/delete > /dev/null

# Test 10: Follow redirects
echo "✓ Testing redirect following..."
./bin/whipl -L http://httpbin.org/redirect/2 > /dev/null

echo
echo "All tests passed!"
