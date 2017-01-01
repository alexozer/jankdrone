package main

import "github.com/alexozer/jankcopter/client"

func main() {
	out := make(chan []client.Var)
	go client.NewHandheld(out).Start()
	go client.NewCli(out).Start()
	go client.NewSender(out).Start()

	select {}
}
