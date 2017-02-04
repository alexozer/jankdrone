package client

import (
	"bufio"
	"fmt"
	"log"
	"os"
	"regexp"
	"strconv"
	"strings"
)

type Cli struct {
	in  <-chan BoundVar
	out chan<- []BoundVar
}

func NewCli(in <-chan BoundVar, out chan<- []BoundVar) *Cli {
	return &Cli{in, out}
}

var cliRegex = regexp.MustCompile(`^(([A-Za-z]+\w*)?\.([A-Za-z]+\w*)?(\s+(\S+))?)|(\S*)$`)

var cliShortcuts = map[string]BoundVar{
	"k": MustBindVar("switches", "softKill", true),
	"u": MustBindVar("switches", "softKill", false),
	"e": MustBindVar("controller", "enabled", true),
	"d": MustBindVar("controller", "enabled", false),

	"ye": MustBindVar("yawConf", "enabled", true),
	"pe": MustBindVar("pitchConf", "enabled", true),
	"re": MustBindVar("rollConf", "enabled", true),
	"yd": MustBindVar("yawConf", "enabled", false),
	"pd": MustBindVar("pitchConf", "enabled", false),
	"rd": MustBindVar("rollConf", "enabled", false),

	"yaw":   MustBindVar("placement", "yaw", nil),
	"pitch": MustBindVar("placement", "pitch", nil),
	"roll":  MustBindVar("placement", "roll", nil),
}

func (this *Cli) Start() {
	go this.read()
	go this.write()
}

func (this *Cli) read() {
	reader := bufio.NewReader(os.Stdin)
	var lastGroup, lastName string

	for {
		fmt.Print("> ")
		input, err := reader.ReadString('\n')
		if err != nil {
			log.Fatal("Could not read from stdin")
		}

		input = strings.TrimSpace(input)

		matches := cliRegex.FindStringSubmatch(input)
		if matches == nil {
			fmt.Println("Invalid format, expected: '[[group].[variable] ]value'")
			continue
		}

		path, group, name := matches[1], matches[2], matches[3]
		var valueStr string
		if len(matches[5]) > 0 {
			valueStr = matches[5]
		} else {
			valueStr = matches[6]
		}

		if shortcut, ok := cliShortcuts[valueStr]; ok && len(path) == 0 {
			if shortcut.Value == nil {
				lastGroup = shortcut.Group
				lastName = shortcut.Name
			}
			this.out <- []BoundVar{shortcut}
			continue
		}

		if len(group) == 0 {
			group = lastGroup
		} else {
			lastGroup = group
		}
		if len(name) == 0 {
			name = lastName
		} else {
			lastName = name
		}

		var value interface{}
		if len(valueStr) > 0 {
			if value, err = strconv.Atoi(valueStr); err != nil {
				if value, err = strconv.ParseFloat(valueStr, 64); err != nil {
					if value, err = strconv.ParseBool(valueStr); err != nil {
						fmt.Println("Error parsing value")
						continue
					}
				}
			}
		}

		boundVar, err := BindVar(group, name, value)
		if err != nil {
			fmt.Println(err)
			continue
		}

		this.out <- []BoundVar{boundVar}
	}
}

func (this *Cli) write() {
	for v := range this.in {
		fmt.Println(v)
	}
}
