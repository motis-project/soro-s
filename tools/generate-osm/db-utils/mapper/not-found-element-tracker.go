package mapper

import "fmt"

type ElementType int

const (
	Halt ElementType = iota
	SpeedLimits
	Slopes
	Eotds
	LineSwitch
	KilometrageJump
	Border
	Bumper
	Tunnel
	TrackEnd
	MainSignal
	ApproachSignal
	ProtectionSignal
	Switches
	Crosses
)

func (elementType ElementType) String() string {
	return [...]string{
		"Halts",
		"Speed Limits",
		"Slopes",
		"Eotds",
		"Line Switchs",
		"Kilometrage Jump",
		"Borders",
		"Bumpers",
		"Tunnels",
		"Track Ends",
		"Main Signals",
		"Approach Signals",
		"Protection Signals",
		"Switches",
		"Crosses",
	}[elementType]
}

type NotFoundElementTracker struct {
	NotFoundElements map[ElementType]([]string)
}

func NewNotFoundElementTracker() NotFoundElementTracker {
	return NotFoundElementTracker{
		NotFoundElements: make(map[ElementType]([]string)),
	}
}

func (tracker *NotFoundElementTracker) AddNotFoundElement(elementType ElementType, elementName string) {
	tracker.NotFoundElements[elementType] = append(tracker.NotFoundElements[elementType], elementName)
}

func (tracker *NotFoundElementTracker) GetNotFoundElemetsCount(elementType ElementType) int {
	return len(tracker.NotFoundElements[elementType])
}

func (tracker *NotFoundElementTracker) PrintNotFoundElements() {
	for elementType, elementNames := range tracker.NotFoundElements {
		fmt.Print("Not found elements of type " + elementType.String() + ": ")
		for _, elementName := range elementNames {
			fmt.Printf(elementName + ", ")
		}
		fmt.Println()
	}
}
