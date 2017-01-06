package client

import (
	"fmt"
	"log"
)

type BoundVar struct {
	*Var
	Value interface{}
}

func BindVar(groupName, varName string, value interface{}) (BoundVar, error) {
	group, ok := Shm[groupName]
	if !ok {
		return BoundVar{}, fmt.Errorf("Group '%s' not found", groupName)
	}

	v, ok := group[varName]
	if !ok {
		return BoundVar{}, fmt.Errorf("Var '%s.%s' not found", groupName, varName)
	}

	var typesMatch bool
	switch v.DefaultValue.(type) {
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
		return BoundVar{}, fmt.Errorf("Types do not match")
	}

	return BoundVar{v, value}, nil
}

func MustBindVar(groupName, varName string, value interface{}) BoundVar {
	v, err := BindVar(groupName, varName, value)
	if err != nil {
		log.Fatal(err)
	}
	return v
}
