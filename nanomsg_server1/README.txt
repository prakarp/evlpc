nanomsg_server1 

Server:
./dist/Debug/GNU-Linux-x86/nanomsg_server1 node0  ipc:///tmp/reqrep.ipc &

Client:
./dist/Debug/GNU-Linux-x86/nanomsg_server1 node1 ipc:///tmp/reqrep2.ipc "WHAT" 100 &

