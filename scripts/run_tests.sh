#!/bin/bash

self="$0"
scriptdir=$(cd `dirname $self`; pwd)
cd $scriptdir/..


function die()
{
    echo "$*" >&2
    exit 1
}

function killhttpd()
{
    if [ -f httpd.pid ]; then
        pid=$(cat httpd.pid)
        kill $pid
        while kill -0 $pid &> /dev/null; do
            sleep 0.01;
        done
        rm -f httpd.pid
    fi
    if [ "x$1" != "x" ]; then
        die $1
    fi
}

function httpget()
{
    echo -n "httpget $*... " >&2
    examples/httpget/bin/httpget $*://localhost:$port/random_data >/dev/null || killhttpd "httpget $* failed"
}

function httpget_compare()
{
    httpget $*
    echo -n "Comparing... " >&2
    cmp transferred_data random_data || killhttpd "$* mismatch"
    echo '[OK]' >&2
}

function run_tests() {
    # test file output routine accuracy
    httpget_compare -o transferred_data $1
    # test memory routine accuracy
    200>transferred_data httpget_compare -f200 $1
    # test "content" pointer usage
    1>/dev/null httpget -pc50 -n200 $1
    echo '[OK]' >&2
    killhttpd
}

function test_http()
{
    echo -n "Staring httpd... " >&2
    examples/httpd/bin/httpd -d -p0 >/dev/null || die "httpd failed to start"
    echo '[OK]' >&2
    port=$(cat httpd.port)
    run_tests http
}

function test_https()
{
    echo -n "Creating server key and certificate... " >&2
    openssl req -new -x509 -days 365 -nodes -out httpd.pem -keyout httpd.key -batch >/dev/null 2>&1 || die "openssl req"
    echo '[OK]' >&2
    echo -n "Starting httpd with SSL enabled... " >&2
    examples/httpd/bin/httpd -d -p0 -s0 -k httpd.key -c httpd.pem -h examples/httpd/dhparam.pem >/dev/null || killhttpd "httpd failed to start with ssl"
    echo '[OK]' >&2
    port=$(cat httpd.sport)
    run_tests https
    rm -f httpd.pem httpd.key
}
make || die "make"
./tests/bin/test_ribs2 || die "unit tests"

ulimit -Ss 4096
# over 2MB to force mremap to move pointers
echo -n "Creating random data data file... " >&2
dd if=/dev/urandom of=random_data bs=1024 count=4048 2>/dev/null || die "couldn't"
echo '[OK]' >&2
test_http
if [ -f /usr/include/openssl/ssl.h ]; then
    test_https
fi
rm -f random_data transferred_data httpd.port httpd.sport httpd.log
