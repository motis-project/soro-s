package mapper

import (
	"encoding/xml"
)

type Wert struct {
	XMLName xml.Name
	Value   string `xml:",chardata"`
}

type Stamm struct {
	XMLName xml.Name `xml:"Stamm"`
	ID      string   `xml:"ID,attr"`
}
type Abzweig struct {
	XMLName xml.Name `xml:"Abzweig"`
	ID      string   `xml:"ID,attr"`
}
type Partner struct {
	XMLName xml.Name
	Name    string `xml:",chardata"`
	ID      string `xml:",attr"`
}

type KnotenTyp struct {
	XMLName        xml.Name
	ID             Wert `xml:"ID"`
	Kilometrierung Wert `xml:"Kilometrierung"`
}

type Weichenanfang struct {
	KnotenTyp
	Name    Wert    `xml:"Name"`
	Stamm   Stamm   `xml:"Stamm"`
	Abzweig Abzweig `xml:"Abzweig"`
}
type Weichenknoten struct {
	KnotenTyp
	Partner Partner `xml:"Partner"`
}

type Kreuzungsknoten struct {
	XMLName xml.Name
	ID      string `xml:"ID,attr"`
}
type KreuzungsweicheAnfangLinks struct {
	KnotenTyp
	Name             Wert            `xml:"NameAnfang"`
	NachbarGeradeaus Kreuzungsknoten `xml:"NachbarGeradeaus"`
	NachbarAbzweig   Kreuzungsknoten `xml:"NachbarAbzweig"`
}
type KreuzungsweicheAnfangRechts struct {
	KnotenTyp
	NachbarGeradeaus  Kreuzungsknoten `xml:"NachbarGeradeaus"`
	HauptknotenAnfang Kreuzungsknoten `xml:"HauptknotenAnfang"`
	HauptknotenEnde   Kreuzungsknoten `xml:"HauptknotenEnde"`
}
type KreuzungsweicheEndeLinks struct {
	KnotenTyp
	NachbarGeradeaus Kreuzungsknoten `xml:"NachbarGeradeaus"`
	NachbarAbzweig   Kreuzungsknoten `xml:"NachbarAbzweig"`
}
type KreuzungsweicheEndeRechts struct {
	KnotenTyp
	NachbarGeradeaus  Kreuzungsknoten `xml:"NachbarGeradeaus"`
	HauptknotenAnfang Kreuzungsknoten `xml:"HauptknotenAnfang"`
	HauptknotenEnde   Kreuzungsknoten `xml:"HauptknotenEnde"`
}

type SimpleElement struct {
	KnotenTyp
}
type NamedSimpleElement struct {
	KnotenTyp
	Name Wert `xml:"Name"`
}

type MaxGeschwindigkeit struct {
	KnotenTyp
	Geschwindigkeit Wert `xml:"Geschw"`
}
type Neigung struct {
	KnotenTyp
	Rising  Wert `xml:"Steigend"`
	Falling Wert `xml:"Fallend"`
}

type Spurplanknoten struct {
	XMLName xml.Name `xml:"Spurplanknoten"`

	BetriebsStGr []*SimpleElement      `xml:"Betriebsstellengrenze"`
	Prellbock    []*SimpleElement      `xml:"Prellbock"`
	Gleisende    []*NamedSimpleElement `xml:"Gleisende"`

	WeichenAbzwLinks  []*Weichenknoten `xml:"WeichenabzweigLinks"`
	WeichenAbzwRechts []*Weichenknoten `xml:"WeichenabzweigRechts"`
	WeichenStamm      []*Weichenknoten `xml:"Weichenstamm"`
	WeichenAnf        []*Weichenanfang `xml:"Weichenanfang"`

	KreuzungsweicheAnfangLinks  []*KreuzungsweicheAnfangLinks  `xml:"KreuzungsweicheAnfangLinks"`
	KreuzungsweicheAnfangRechts []*KreuzungsweicheAnfangRechts `xml:"KreuzungsweicheAnfangRechts"`
	KreuzungsweicheEndeLinks    []*KreuzungsweicheEndeLinks    `xml:"KreuzungsweicheEndeLinks"`
	KreuzungsweicheEndeRechts   []*KreuzungsweicheEndeRechts   `xml:"KreuzungsweicheEndeRechts"`

	HauptsigF  []*NamedSimpleElement `xml:"HauptsignalF"`
	HauptsigS  []*NamedSimpleElement `xml:"HauptsignalS"`
	VorsigF    []*NamedSimpleElement `xml:"VorsignalF"`
	VorsigS    []*NamedSimpleElement `xml:"VorsignalS"`
	SchutzsigF []*NamedSimpleElement `xml:"SchutzsignalF"`
	SchutzsigS []*NamedSimpleElement `xml:"SchutzsignalS"`

	HalteplGzF []*NamedSimpleElement `xml:"HalteplatzGzF"`
	HalteplGzS []*NamedSimpleElement `xml:"HalteplatzGzS"`
	HalteplRzF []*NamedSimpleElement `xml:"HalteplatzRzF"`
	HalteplRzS []*NamedSimpleElement `xml:"HalteplatzRzS"`

	KmSprungAnf  []*SimpleElement `xml:"KmSprungAnfang"`
	KmSprungEnde []*SimpleElement `xml:"KmSprungEnde"`

	MaxSpeedF []*MaxGeschwindigkeit `xml:"GeschwZulaessigF"`
	MaxSpeedS []*MaxGeschwindigkeit `xml:"GeschwZulaessigS"`

	Neigung []*Neigung            `xml:"Neigung"`
	Tunnel  []*NamedSimpleElement `xml:"Tunnel"`

	SignalZugschlussstelleF []*SimpleElement `xml:"SignalZugschlussstelleF"`
	SignalZugschlussstelleS []*SimpleElement `xml:"SignalZugschlussstelleS"`
	FstrZugschlussstelleF   []*SimpleElement `xml:"FstrZugschlussstelleF"`
	FstrZugschlussstelleS   []*SimpleElement `xml:"FstrZugschlussstelleS"`

	Streckenwechsel0 []*SimpleElement `xml:"Streckenwechsel0"`
	Streckenwechsel1 []*SimpleElement `xml:"Streckenwechsel1"`
}

type Strecke struct {
	XMLName xml.Name `xml:"Strecke"`
	Nummer  string   `xml:",chardata"`
}

type Betriebsstelle struct {
	XMLName xml.Name `xml:"Betriebsstelle"`
	Name    string   `xml:",chardata"`
}

type Spurplanabschnitt struct {
	XMLName    xml.Name          `xml:"Spurplanabschnitt"`
	StreckenNr Strecke           `xml:"Strecke"`
	Knoten     []*Spurplanknoten `xml:"Spurplanknoten"`
}

type Spurplanbetriebsstelle struct {
	XMLName    xml.Name             `xml:"Spurplanbetriebsstelle"`
	Name       []*Betriebsstelle    `xml:"Betriebsstelle"`
	Abschnitte []*Spurplanabschnitt `xml:"Spurplanabschnitte>Spurplanabschnitt"`
}

type XmlIssDaten struct {
	XMLName         xml.Name                  `xml:"XmlIssDaten"`
	Betriebsstellen []*Spurplanbetriebsstelle `xml:"Spurplanbetriebsstellen>Spurplanbetriebsstelle"`
}
