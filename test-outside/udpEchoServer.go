package main

import (
	"fmt"
	"net"
)

const listenPort = "55555"

func main() {
	/* Lets prepare a address at any address at port 55555*/
	ServerAddr, err := net.ResolveUDPAddr("udp", fmt.Sprintf(":%s", listenPort))
	if err != nil {
		fmt.Println("resolve udp address failed")
		return
	}
	fmt.Println("udp echo server listening on ", listenPort)

	/* Now listen at selected port */
	ServerConn, err := net.ListenUDP("udp", ServerAddr)
	if err != nil {
		fmt.Println("listen udp ", ServerAddr, "failed")
		return
	}
	defer ServerConn.Close()

	buf := make([]byte, 1024)

	for {
		 n, addr, err := ServerConn.ReadFromUDP(buf)
		 fmt.Printf("received: %s from: %s\n", string(buf[0:n]), addr)

		if err != nil {
			fmt.Println("error: ", err)
		}

		ServerConn.WriteTo(buf[0:n], addr)
	}
}
