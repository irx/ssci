#!/bin/sh -e
# test exemplary server using netcat

make example-server
serv_in="$(mktemp)"
serv_out="$(mktemp)"

cleanup()
{
	echo q > "${serv_in}"
	rm "${serv_in}" "${serv_out}"
}

trap cleanup EXIT

echo "servers std io will be piped thru tmp files"
echo " stdin:  ${serv_in}"
echo " stdout: ${serv_out}"
./example-server < "${serv_in}" > "${serv_out}" &
sleep 1

msg="hello"
echo
echo "sending message \`${msg}' with netcat..."
echo "${msg}" | nc -w 1 -N localhost 1337
echo "looking for message \`${msg}' in servers stdout:"
grep "${msg}" "${serv_out}"
