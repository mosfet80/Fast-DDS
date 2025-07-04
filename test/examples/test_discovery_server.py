# Copyright 2024 Proyectos y Sistemas de Mantenimiento SL (eProsima).
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

import subprocess
import os
import pytest
import random

"""
    Expected output test cases for Discovery Server example.
    Each element is a ternary (pub_args, sub_args, server_args)
"""
discovery_server_test_cases = []

# IPv6 in docker is not supported on windows hosts
# https://docs.docker.com/engine/daemon/ipv6/
if os.name == 'nt':
    discovery_server_test_cases = [
        ('--samples 10 --connection-address 172.150.0.4', '--samples 10 --connection-address 172.150.0.4', '--timeout 15 --listening-address 172.150.0.4', '--listening-port 11400 --timeout 1 --listening-address 172.150.0.5'),
        ('--connection-port 11500 --samples 10 --connection-address 172.150.0.4', '--connection-port 11500 --samples 10 --connection-address 172.150.0.4', '--listening-port 11500 --timeout 15 --listening-address 172.150.0.4', '--listening-port 11400 --timeout 1 --listening-address 172.150.0.5'),
        ('--transport tcpv4 --samples 10 --connection-address 172.150.0.4', '--transport tcpv4 --samples 10 --connection-address 172.150.0.4', '--transport tcpv4 --timeout 15 --listening-address 172.150.0.4', '--listening-port 11400 --timeout 1 --listening-address 172.150.0.5'),
        ('--samples 10 --connection-address 172.150.0.4', '--connection-port 11400 --samples 10 --connection-address 172.150.0.5', '--timeout 15 --connection-port 11400 --connection-address 172.150.0.5 --listening-address 172.150.0.4', '--listening-port 11400 --timeout 15 --listening-address 172.150.0.5')
    ]
else:
    discovery_server_test_cases = [
        ('--samples 10 --connection-address 172.150.0.4', '--samples 10 --connection-address 172.150.0.4', '--timeout 8 --listening-address 172.150.0.4', '--listening-port 11400 --timeout 1 --listening-address 172.150.0.5'),
        ('--connection-port 11500 --samples 10 --connection-address 172.150.0.4', '--connection-port 11500 --samples 10 --connection-address 172.150.0.4', '--listening-port 11500 --timeout 8 --listening-address 172.150.0.4', '--listening-port 11400 --timeout 1 --listening-address 172.150.0.5'),
        ('--transport tcpv4 --samples 10 --connection-address 172.150.0.4', '--transport tcpv4 --samples 10 --connection-address 172.150.0.4', '--transport tcpv4 --timeout 8 --listening-address 172.150.0.4', '--listening-port 11400 --timeout 1 --listening-address 172.150.0.5'),
        ('--samples 10 --connection-address 172.150.0.4', '--connection-port 11400 --samples 10 --connection-address 172.150.0.5', '--timeout 8 --connection-port 11400 --connection-address 172.150.0.5 --listening-address 172.150.0.4', '--listening-port 11400 --timeout 8 --listening-address 172.150.0.5'),
        ('--transport tcpv4 --samples 10  --connection-address 172.150.0.4', '--transport tcpv4 --connection-port 18000 --samples 10 --connection-address 172.150.0.5', '--transport tcpv4 --timeout 8 --connection-port 18000 --connection-address 172.150.0.5 --listening-address 172.150.0.4', '--transport tcpv4 --listening-port 18000 --timeout 8 --listening-address 172.150.0.5'),
        #Disable IPv6 tests in CI
        #('--transport udpv6 --samples 10 --connection-address 2001:3984:3989::4', '--transport udpv6 --connection-port 18000 --samples 10 --connection-address 2001:3984:3989::5', '--transport udpv6 --timeout 8 --connection-port 18000 --connection-address 2001:3984:3989::5 --listening-address 2001:3984:3989::4', '--transport udpv6 --listening-port 18000 --timeout 8 --listening-address 2001:3984:3989::5'),
        # ('--transport tcpv6 --samples 10 --connection-address 2001:3984:3989::4', '--transport tcpv6 --connection-port 18000 --samples 10 --connection-address 2001:3984:3989::5', '--transport tcpv6 --timeout 8 --connection-port 18000 --connection-address 2001:3984:3989::5 --listening-address 2001:3984:3989::4', '--transport tcpv6 --listening-port 18000 --timeout 8 --listening-address 2001:3984:3989::5')
    ]

@pytest.mark.parametrize("pub_args, sub_args, server1_args, server2_args", discovery_server_test_cases)
def test_discovery_server(pub_args, sub_args, server1_args, server2_args):
    """."""
    ret = False
    out = ''

    menv = dict(os.environ)

    menv["PUB_ARGS"] =  pub_args
    menv["SUB_ARGS"] =  sub_args
    menv["SERVER1_ARGS"] =  server1_args
    menv["SERVER2_ARGS"] =  server2_args

    # Set a random suffix to the cotainer name to avoid conflicts with other containers
    menv["CONTAINER_SUFFIX_COMPOSE"] = str(random.randint(0,100))

    timeout = 30
    #In windows timeout argument does not wor properly
    #It is not able to kill the subprocess when it is surpassed
    #and when the subprocess exists then it is checked.
    #This test is about samples (1000) so for big msgs in windows
    #it can take longer. Set it to a higher value
    if os.name == 'nt':
        timeout = 200

    try:
        out = subprocess.check_output(
            '"@DOCKER_EXECUTABLE@" compose -f discovery_server.compose.yml up',
            stderr=subprocess.STDOUT,
            shell=True,
            timeout=timeout,
            env=menv
        ).decode().split('\n')

        sent = 0
        received = 0
        for line in out:
            if 'SENT' in line:
                sent += 1
                continue

            if 'RECEIVED' in line:
                received += 1
                continue

        if sent != 0 and received != 0 and sent == received:
            ret = True
        else:
            print('ERROR: sent: ' + str(sent) + ', but received: ' + str(received))
            raise subprocess.CalledProcessError(1, '')

    except subprocess.CalledProcessError:
        for l in out:
            print(l)
    except subprocess.TimeoutExpired:
        print('TIMEOUT')
        print(out)

    # Cleanup resources
    subprocess.call('"@DOCKER_EXECUTABLE@" compose -f discovery_server.compose.yml down', shell=True)

    assert(ret)
