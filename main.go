package main

import (
	"bufio"
	"fmt"
	"log"
	"os/exec"
	"time"
)

func main() {
	fmt.Println("start")

	ch := make(chan string)

	go sensoring(ch)

	for {
		select {
		case sencsorVal := <-ch:
			fmt.Println("sensor's value is " + sencsorVal)
			ExecuteCmd("おかえりなさい。")
			time.Sleep(5 * time.Second)
			go sensoring(ch)
		}
	}
}

// starting to watch sensor.
func sensoring(ch chan string) {

	cmdPath := " E:/ProgramFiles/dev/go/src/StandByMe/iws600cm/windows/iws600cm.exe loop ANY"
	// cmdPath := "iws600cm"
	// sensorOpt := "loop ANY"
	// test := "test.bat"

	// for windows command.
	cmd := exec.Command("cmd", "/C", cmdPath) //, sensorOpt)

	stdout, err := cmd.StdoutPipe()

	if err != nil {
		fmt.Println("erorr...")
		fmt.Println(err)
		return
	}

	fmt.Println("sensor start...")
	cmd.Start()

	var line string

	scanner := bufio.NewScanner(stdout)
	for scanner.Scan() {
		line = scanner.Text()
		fmt.Println("out : " + line)
		if line == "2" {
			break
		}
	}

	ch <- line

}

func ExecuteCmd(manuscript string) {

	voiceroidPath := "E:/ProgramFiles/510product/SeikaSay/seikasay.exe"
	cID := "2002"
	command := voiceroidPath + " -cid " + cID + " -t " + manuscript
	//実行するコマンドを設定
	cmd := exec.Command("cmd", "/C", command)
	//コマンドを実行するときのカレントディレクトリを設定
	// cmd.Dir = "コマンドを実行するときのカレントディレクトリ"

	//コマンドを実行開始
	if err := cmd.Start(); err != nil {
		log.Printf("err: %s", err)
	}
}
