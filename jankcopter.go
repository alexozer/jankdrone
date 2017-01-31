package main

import "github.com/alexozer/jankcopter/client"

func main() {
	out, in := make(chan []client.BoundVar), make(chan client.BoundVar)
	status := make(chan string)
	client.NewHandheld(out, status).Start()
	client.NewCli(in, out, status).Start()
	client.NewSender(out, in, status).Start()

	select {}
}
