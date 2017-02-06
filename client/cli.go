package client

import (
	"fmt"
	"log"
	"os"
	"regexp"
	"strconv"
	"strings"
	"time"

	ui "github.com/gizak/termui"
)

const (
	boxWidth, boxHeight = 16, 9
	quickViewWidth      = 8
	cmdWidth, cmdHeight = 3*(boxWidth+1) + quickViewWidth, 3

	maxInputLen = cmdWidth - 3

	statusLines = 5
)

type Cli struct {
	in     <-chan BoundVar
	out    chan<- []BoundVar
	status chan string

	vars map[string]map[string]BoundVar

	inputRunes          []rune
	lastGroup, lastName string
}

func NewCli(in <-chan BoundVar, out chan<- []BoundVar, status chan string) *Cli {
	this := &Cli{
		in:     in,
		out:    out,
		status: status,
		vars:   make(map[string]map[string]BoundVar),
	}

	this.addShmGroup("placement")
	this.addShmGroup("yawSettings")
	this.addShmGroup("pitchSettings")
	this.addShmGroup("rollSettings")
	this.addShmGroup("desires")
	this.addShmGroup("controller")
	this.addShmGroup("controllerOut")
	this.addShmVar("switches", "softKill")
	this.addShmGroup("power")

	return this
}

func (this *Cli) addShmVar(group, name string) {
	if _, ok := this.vars[group]; !ok {
		this.vars[group] = make(map[string]BoundVar)
	}

	v := MustBindVarTag(Shm[group][name].Tag, nil)
	v.Value = v.DefaultValue
	this.vars[group][name] = v
}

func (this *Cli) addShmGroup(name string) {
	for k := range Shm[name] {
		this.addShmVar(name, k)
	}
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
	//go this.read()
	//go this.write()
	go this.run()
}

func (this *Cli) run() {
	if err := ui.Init(); err != nil {
		log.Fatal(err)
	}

	ui.Handle("/sys/kbd/C-c", func(ui.Event) {
		ui.StopLoop()
		ui.Close()
		os.Exit(0)
	})
	ui.Handle("/sys/kbd", func(e ui.Event) {
		this.handleInput(e.Data.(ui.EvtKbd).KeyStr)
	})
	go ui.Loop()
	go this.drawStatus()

	this.drawCommand()
	this.drawVars()

	//const drawPeriod, pollPeriod = time.Second / 15, time.Second / 10
	const drawPeriod, pollPeriod = time.Second / 15, time.Second * 2
	drawChan, pollChan := time.After(drawPeriod), time.After(pollPeriod)
	//drawChan := time.After(drawPeriod)
	needRedraw := true

	for {
		select {
		case v := <-this.in:
			this.vars[v.Group][v.Name] = v
			needRedraw = true

		case <-drawChan:
			if needRedraw {
				this.drawVars()
				needRedraw = false
			}

			drawChan = time.After(drawPeriod)

		case <-pollChan:
			varReqs := make([]BoundVar, 0)
			for _, g := range this.vars {
				for _, v := range g {
					varReqs = append(varReqs, BoundVar{v.Var, nil})
				}
			}
		}

	}
}

func (this *Cli) handleInput(inputKey string) {
	if len(inputKey) == 1 && len(this.inputRunes) < maxInputLen {
		this.inputRunes = append(this.inputRunes, []rune(inputKey)...)

	} else if inputKey == "<space>" && len(this.inputRunes) < maxInputLen {
		this.inputRunes = append(this.inputRunes, ' ')

	} else if inputKey == "C-8" && len(this.inputRunes) > 0 {
		this.inputRunes = this.inputRunes[:len(this.inputRunes)-1]

	} else if inputKey == "<enter>" {
		this.processCommand()
		this.inputRunes = this.inputRunes[:0]
	}

	this.drawCommand()
}

func (this *Cli) processCommand() {
	trimmed := strings.TrimSpace(string(this.inputRunes))
	matches := cliRegex.FindStringSubmatch(trimmed)
	if matches == nil {
		this.status <- "Invalid format, expected: '[[group].[variable] ]value'"
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
			this.lastGroup = shortcut.Group
			this.lastName = shortcut.Name
		}
		this.out <- []BoundVar{shortcut}
		return
	}

	if len(group) == 0 {
		group = this.lastGroup
	} else {
		this.lastGroup = group
	}
	if len(name) == 0 {
		name = this.lastName
	} else {
		this.lastName = name
	}

	var value interface{}
	var err error
	if len(valueStr) > 0 {
		if value, err = strconv.Atoi(valueStr); err != nil {
			if value, err = strconv.ParseFloat(valueStr, 64); err != nil {
				if value, err = strconv.ParseBool(valueStr); err != nil {
					this.status <- "Error parsing value"
					return
				}
			}
		}
	}

	boundVar, err := BindVar(group, name, value)
	if err != nil {
		this.status <- err.Error()
		return
	}

	this.out <- []BoundVar{boundVar}
}

func padApart(s1, s2 string, width int) string {
	padWidth := width - len(s1) - len(s2)
	if padWidth <= 0 {
		return (s1 + s2)[:width]
	}

	pad := make([]rune, padWidth)
	for i := range pad {
		pad[i] = ' '
	}

	return s1 + string(pad) + s2
}

// It appears drawing with termui may produce race conditions, however this is
// currently an inherent property of termui and cannot easily be avoided by the
// client. See https://github.com/gizak/termui/issues/122
func (this *Cli) drawVars() {
	this.drawAxis("yaw", 0, 0, boxWidth, boxHeight)
	this.drawAxis("pitch", boxWidth+1, 0, boxWidth, boxHeight)
	this.drawAxis("roll", 2*(boxWidth+1), 0, boxWidth, boxHeight)
	this.drawQuickView()
}

func (this *Cli) drawAxis(name string, x, y, width, height int) {
	ls := ui.NewList()

	settings := this.vars[name+"Settings"]
	textWidth := width - 3
	ls.Items = []string{
		padApart("DES:", this.vars["desires"][name].ValueString(), textWidth),
		padApart("VAL:", this.vars["placement"][name].ValueString(), textWidth),
		padApart("OUT:", this.vars["controllerOut"][name].ValueString(), textWidth),
		padApart("P:", settings["p"].ValueString(), textWidth),
		padApart("I:", settings["i"].ValueString(), textWidth),
		padApart("D:", settings["d"].ValueString(), textWidth),
		padApart("EN:", settings["enabled"].ValueString(), textWidth),
	}

	ls.X = x
	ls.Y = y
	ls.Width = width
	ls.Height = height
	ls.BorderLabel = name

	ui.Render(ls)
}

func (this *Cli) drawQuickView() {
	colorBool := func(s string, b bool) string {
		if b {
			return fmt.Sprintf("[%s](fg-black,bg-white)", s)
		}
		return s
	}
	colorize := func(s, color string) string {
		return fmt.Sprintf("[%s](bg-%s)", s, color)
	}

	voltageStr := fmt.Sprintf("%.2fV", this.vars["power"]["voltage"].Value.(float64))
	if this.vars["power"]["critical"].Value.(bool) {
		voltageStr = colorize(voltageStr, "yellow")
	} else if this.vars["power"]["low"].Value.(bool) {
		voltageStr = colorize(voltageStr, "red")
	}

	softKill := this.vars["switches"]["softKill"].Value.(bool)
	controllerEnabled := this.vars["controller"]["enabled"].Value.(bool)

	qv := ui.NewPar(fmt.Sprintf("%s\n%s\n%s\n%s",
		// TODO implement sync toggle
		colorBool("  SY  ", false),
		colorBool("  SK  ", softKill),
		colorBool("  EN  ", controllerEnabled),
		voltageStr,
	))
	qv.X, qv.Y = (boxWidth+1)*3, 0
	qv.Width, qv.Height = quickViewWidth, boxHeight

	ui.Render(qv)
}

func (this *Cli) drawCommand() {
	const cursor = "[ ](bg-white)"

	par := ui.NewPar(string(this.inputRunes) + cursor)
	par.X, par.Y = 0, boxHeight
	par.Width, par.Height = cmdWidth, cmdHeight
	par.BorderLabel = "command"
	ui.Render(par)
}

func (this *Cli) drawStatus() {
	statusQueue := make([]string, 0, statusLines)

	draw := func() {
		st := ui.NewList()
		st.Items = statusQueue
		st.BorderLabel = "status"
		st.X, st.Y = 0, boxHeight+cmdHeight
		st.Width, st.Height = cmdWidth, cap(statusQueue)+2

		ui.Render(st)
	}

	draw()
	for statusStr := range this.status {
		if len(statusQueue) < cap(statusQueue) {
			statusQueue = append(statusQueue, statusStr)
		} else {
			copy(statusQueue[:len(statusQueue)-1], statusQueue[1:])
			statusQueue[len(statusQueue)-1] = statusStr
		}
		draw()
	}
}
