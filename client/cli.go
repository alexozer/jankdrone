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
	out chan<- []Var
}

func NewCli(out chan<- []Var) *Cli {
	return &Cli{out: out}
}

var cliRegex = regexp.MustCompile(`^(([A-Za-z]*)\.([A-Za-z]*)\s+)?(\S+)$`)

var cliShortcuts = map[string]Var{
	"k": MustBindVar("switches", "softKill", true),
	"u": MustBindVar("switches", "softKill", false),
	"e": MustBindVar("controller", "enabled", true),
	"d": MustBindVar("controller", "enabled", false),
}

func (this *Cli) Start() {
	reader := bufio.NewReader(os.Stdin)
	var lastGroup, lastVariable string

	for {
		fmt.Print("> ")
		input, err := reader.ReadString('\n')
		if err != nil {
			log.Fatal("Could not read from stdin")
		}

		input = strings.TrimSpace(input)
		if len(input) == 0 {
			continue
		}

		submatches := cliRegex.FindStringSubmatch(input)
		if submatches == nil {
			fmt.Println("Invalid format, expected: '[[group].[variable] ]value'")
			continue
		}

		var boundVar Var
		boundVar, ok := cliShortcuts[submatches[4]]
		if len(submatches[1]) > 0 || !ok {
			group, variable := submatches[2], submatches[3]
			if len(group) == 0 {
				group = lastGroup
			} else {
				lastGroup = group
			}
			if len(variable) == 0 {
				variable = lastVariable
			} else {
				lastVariable = variable
			}

			var value interface{}
			if value, err = strconv.Atoi(submatches[4]); err != nil {
				if value, err = strconv.ParseFloat(submatches[4], 64); err != nil {
					if value, err = strconv.ParseBool(submatches[4]); err != nil {
						fmt.Println("Error parsing value")
						continue
					}
				}
			}

			boundVar, err = BindVar(group, variable, value)
			if err != nil {
				fmt.Println(err)
				continue
			}
		}

		this.out <- []Var{boundVar}
	}
}
