package client

import (
	"fmt"
	"log"
)

type BoundVar struct {
	*Var
	Value interface{}
}

func (this BoundVar) String() string {
	return fmt.Sprintf("%s.%s = %s", this.Group, this.Name, this.ValueString())
}

func (this BoundVar) ValueString() string {
	switch value := this.Value.(type) {
	case float64:
		return fmt.Sprintf("%.3f", value)
	default:
		return fmt.Sprintf("%v", value)
	}
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

	return bindVar(v, value)
}

func MustBindVar(groupName, varName string, value interface{}) BoundVar {
	v, err := BindVar(groupName, varName, value)
	if err != nil {
		log.Fatal(err)
	}
	return v
}

func BindVarTag(tag int, value interface{}) (BoundVar, error) {
	if tag < 0 || tag >= len(ShmByTag) {
		return BoundVar{}, fmt.Errorf("Var with tag {} not found", tag)
	}
	return bindVar(ShmByTag[tag], value)
}

func MustBindVarTag(tag int, value interface{}) BoundVar {
	v, err := BindVarTag(tag, value)
	if err != nil {
		log.Fatal(err)
	}
	return v
}

func bindVar(v *Var, value interface{}) (BoundVar, error) {
	var typesMatch bool

	if value != nil {
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
	}

	return BoundVar{v, value}, nil
}
