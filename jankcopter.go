package main

import "github.com/alexozer/jankcopter/client"

func main() {
	out, in := make(chan []client.BoundVar), make(chan client.BoundVar)
	client.NewHandheld(out).Start()
	client.NewCli(in, out).Start()
	client.NewSender(out, in).Start()

	select {}
}
