package main

import (
	"encoding/json"
	"os"
)

type Operation struct {
	Kind     string `json:"kind"`
	Quantity int64  `json:"quantity"`
}

func simulate(money, wire int64, operations []Operation) map[string]int64 {
	clips := int64(0)
	for _, operation := range operations {
		quantity := operation.Quantity
		if quantity <= 0 {
			continue
		}
		switch operation.Kind {
		case "buy":
			if money >= quantity {
				money -= quantity
				wire += quantity
			}
		case "make":
			if wire >= quantity {
				wire -= quantity
				clips += quantity
			}
		case "sell":
			if clips >= quantity {
				clips -= quantity
				money += 2 * quantity
			}
		}
	}
	return map[string]int64{"money": money, "wire": wire, "clips": clips}
}

func main() {
	var input struct {
		Money      int64       `json:"money"`
		Wire       int64       `json:"wire"`
		Operations []Operation `json:"operations"`
	}
	if err := json.NewDecoder(os.Stdin).Decode(&input); err != nil {
		panic(err)
	}
	if err := json.NewEncoder(os.Stdout).Encode(simulate(input.Money, input.Wire, input.Operations)); err != nil {
		panic(err)
	}
}
