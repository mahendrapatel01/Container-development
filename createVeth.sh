#!/bin/bash

#    # Init for persistent mount namespaces
#     mkdir /run/mntns \
#         && mount --bind --make-private /run/mntns /run/mntns

#     NSNAME=blue2

#     # Create persistent network+mount namespaces
#     ip netns add ${NSNAME} \
#         && touch "/run/mntns/${NSNAME}" \
#         && ip netns exec "${NSNAME}" \
#             sh -x -c "nsenter -t $$ --mount mount --bind /proc/\$\$/ns/mnt '/run/mntns/${NSNAME}'"

#     # Do NOT use `ip netns exec`
#     # (Re)enter the namespaces with `nsenter`, mounts are persistent
#     nsenter --net=/run/netns/${NSNAME} --mount=/run/mntns/${NSNAME} /bin/bash

NSNAME="netns$1"
VETHH="v-eth-$1"
PEERC="v-peer-$1"

CIP="10.200.$2.31/24"
HIP="10.200.$2.42/24"

echo "netns ${NSNAME}"
echo "v-eth ${VETHH}"
echo "v-peer ${PEERC}"
echo "CIP ${CIP}"
echo "HIP ${HIP}"

ip netns add ${NSNAME}
ip link add ${VETHH} type veth peer name ${PEERC}
ip link set ${PEERC} netns ${NSNAME}
ip addr add ${HIP} dev ${VETHH}
ip link set ${VETHH} up

ip netns exec ${NSNAME} ip addr add ${CIP} dev ${PEERC}
ip netns exec ${NSNAME} ip link set ${PEERC} up
ip netns exec ${NSNAME} ip link set lo up
ip netns exec ${NSNAME} ip route add default via ${HIP}

echo 1 > /proc/sys/net/ipv4/ip_forward

# Flush forward rules, policy DROP by default.
iptables -P FORWARD DROP
iptables -F FORWARD

# Flush nat rules.
iptables -t nat -F

# Enable masquerading of 10.200.1.0.
iptables -t nat -A POSTROUTING -s 10.200.1.0/255.255.255.0 -o eth0 -j MASQUERADE

# Allow forwarding between eth0 and v-eth1.
iptables -A FORWARD -i eth0 -o v-eth1 -j ACCEPT
iptables -A FORWARD -o eth0 -i v-eth1 -j ACCEPT

echo "finished"