package client

import (
	"fmt"
	"log"
)

func BindVar(groupName, varName string, value interface{}) (Var, error) {
	group, ok := Shm[groupName]
	if !ok {
		return Var{}, fmt.Errorf("Group '%s' not found", groupName)
	}

	v, ok := group[varName]
	if !ok {
		return Var{}, fmt.Errorf("Var '%s.%s' not found", groupName, varName)
	}

	var typesMatch bool
	switch v.Value.(type) {
	case float64:
		if intVal, ok := value.(int); ok {
			value = float64(intVal)
			typesMatch = true
		} else {
			_, typesMatch = value.(float64)
		}
	case int:
		_, typesMatch = value.(int)
	case bool:
		_, typesMatch = value.(bool)
	default:
		typesMatch = false
	}
	if !typesMatch {
		return Var{}, fmt.Errorf("Types do not match")
	}

	return Var{value, v.Tag}, nil
}

func MustBindVar(groupName, varName string, value interface{}) Var {
	v, err := BindVar(groupName, varName, value)
	if err != nil {
		log.Fatal(err)
	}
	return v
}
