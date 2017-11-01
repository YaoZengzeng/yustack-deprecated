package main

import (
    "os"
    "fmt"
    "net"
)

const (
    PORT = "222"
    ADDRESS = "10.0.0.2"
)

func main() {
    conn, err := net.Dial("udp", fmt.Sprintf("%s:%s", ADDRESS, PORT))
    defer conn.Close()
    if err != nil {
        os.Exit(1)  
    }

    conn.Write([]byte("Hello world!"))  

    fmt.Println("send msg")

    var msg [20]byte
    conn.Read(msg[0:])

    fmt.Println("receive msg", string(msg[0:10]))

    return
}