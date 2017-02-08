package main

import "github.com/alexozer/jankdrone/client"

func main() {
	out, in := make(chan []client.BoundVar), make(chan client.BoundVar)
	status := make(chan string)
	client.NewCli(in, out, status).Start()
	client.NewSender(out, in, status).Start()

	select {}
}
