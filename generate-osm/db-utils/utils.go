package dbUtils

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
	ID      string `xml:",attr"`
}

type KnotenTyp struct {
	XMLName        xml.Name
	ID             []*Wert `xml:"ID"`
	Kilometrierung []*Wert `xml:"Kilometrierung"`
}

type Weichenanfang struct {
	KnotenTyp
	Stamm   []*Stamm   `xml:"Stamm"`
	Abzweig []*Abzweig `xml:"Abzweig"`
}
type Weichenknoten struct {
	KnotenTyp
	Partner []*Partner `xml:"Partner"`
}

type Kreuzungsknoten struct {
	XMLName xml.Name
	ID      string `xml:"ID,attr"`
}

type KreuzungsweicheAnfangLinks struct {
	KnotenTyp
	Name             []*Wert            `xml:"NameAnfang"`
	NachbarGeradeaus []*Kreuzungsknoten `xml:"NachbarGeradeaus"`
	NachbarAbzweig   []*Kreuzungsknoten `xml:"NachbarAbzweig"`
}
type KreuzungsweicheAnfangRechts struct {
	KnotenTyp
	NachbarGeradeaus  []*Kreuzungsknoten `xml:"NachbarGeradeaus"`
	HauptknotenAnfang []*Kreuzungsknoten `xml:"HauptknotenAnfang"`
	HauptknotenEnde   []*Kreuzungsknoten `xml:"HauptknotenEnde"`
}
type KreuzungsweicheEndeLinks struct {
	KnotenTyp
	NachbarGeradeaus []*Kreuzungsknoten `xml:"NachbarGeradeaus"`
	NachbarAbzweig   []*Kreuzungsknoten `xml:"NachbarAbzweig"`
}
type KreuzungsweicheEndeRechts struct {
	KnotenTyp
	NachbarGeradeaus  []*Kreuzungsknoten `xml:"NachbarGeradeaus"`
	HauptknotenAnfang []*Kreuzungsknoten `xml:"HauptknotenAnfang"`
	HauptknotenEnde   []*Kreuzungsknoten `xml:"HauptknotenEnde"`
}

type KmSprungAnfang struct {
	KnotenTyp
	Partner []*Partner `xml:"Partnerknoten"`
}
type KmSprungEnde struct {
	KnotenTyp
	Partner []*Partner `xml:"Partnerknoten"`
}

type Streckenwechsel0 struct {
	KnotenTyp
	Partner []*Partner `xml:"Partnerknoten"`
}
type Streckenwechsel1 struct {
	KnotenTyp
	Partner []*Partner `xml:"Partnerknoten"`
}

type MaxGeschwindigkeit struct {
	KnotenTyp
	Geschwindigkeit []*Wert `xml:"Geschw"`
}

type Betriebsstellengrenze struct {
	KnotenTyp
	Partner []*Wert `xml:"PartnerBtrst"`
}
type Gleisende struct {
	KnotenTyp
	Name []*Wert `xml:"Name"`
}
type Prellbock struct {
	KnotenTyp
}

type Signal struct {
	KnotenTyp
	Name []*Wert `xml:"Name"`
}

type Halteplatz struct {
	KnotenTyp
	Name []*Wert `xml:"Name"`
}

type Zugschlussstelle struct {
	KnotenTyp
}

type Neigung struct {
	KnotenTyp
	Rising  []*Wert `xml:"Steigend"`
	Falling []*Wert `xml:"Fallend"`
}
type Tunnel struct {
	KnotenTyp
	Name []*Wert `xml:"Name"`
}

type Spurplanknoten struct {
	XMLName xml.Name `xml:"Spurplanknoten"`

	BetriebsStGr []*Betriebsstellengrenze `xml:"Betriebsstellengrenze"`
	Prellbock    []*Prellbock             `xml:"Prellbock"`
	Gleisende    []*Gleisende             `xml:"Gleisende"`

	WeichenAbzwLinks  []*Weichenknoten `xml:"WeichenabzweigLinks"`
	WeichenAbzwRechts []*Weichenknoten `xml:"WeichenabzweigRechts"`
	WeichenStamm      []*Weichenknoten `xml:"WeichenStamm"`
	WeichenAnf        []*Weichenanfang `xml:"Weichenanfang"`

	KreuzungsweicheAnfangLinks  []*KreuzungsweicheAnfangLinks  `xml:"KreuzungsweicheAnfangLinks"`
	KreuzungsweicheAnfangRechts []*KreuzungsweicheAnfangRechts `xml:"KreuzungsweicheAnfangRechts"`
	KreuzungsweicheEndeLinks    []*KreuzungsweicheEndeLinks    `xml:"KreuzungsweicheEndeLinks"`
	KreuzungsweicheEndeRechts   []*KreuzungsweicheEndeRechts   `xml:"KreuzungsweicheEndeRechts"`

	HauptsigF  []*Signal `xml:"HauptsignalF"`
	HauptsigS  []*Signal `xml:"HauptsignalS"`
	VorsigF    []*Signal `xml:"VorsignalF"`
	VorsigS    []*Signal `xml:"VorsignalS"`
	SchutzsigF []*Signal `xml:"SchutzsignalF"`
	SchutzsigS []*Signal `xml:"SchutzsignalS"`

	HalteplGzF []*Halteplatz `xml:"HalteplatzGzF"`
	HalteplGzS []*Halteplatz `xml:"HalteplatzGzS"`
	HalteplRzF []*Halteplatz `xml:"HalteplatzRzF"`
	HalteplRzS []*Halteplatz `xml:"HalteplatzRzS"`

	KmSprungAnf  []*KmSprungAnfang `xml:"KmSprungAnfang"`
	KmSprungEnde []*KmSprungEnde   `xml:"KmSprungEnde"`

	MaxSpeedF []*MaxGeschwindigkeit `xml:"GeschwZulaessigF"`
	MaxSpeedS []*MaxGeschwindigkeit `xml:"GeschwZulaessigS"`

	Neigung []*Neigung `xml:"Neigung"`
	Tunnel  []*Tunnel  `xml:"Tunnel"`

	SignalZugschlussstelleF []*Zugschlussstelle `xml:"SignalZugschlussstelleF"`
	SignalZugschlussstelleS []*Zugschlussstelle `xml:"SignalZugschlussstelleS"`
	FstrZugschlussstelleF   []*Zugschlussstelle `xml:"FstrZugschlussstelleF"`
	FstrZugschlussstelleS   []*Zugschlussstelle `xml:"FstrZugschlussstelleS"`

	Streckenwechsel0 []*Streckenwechsel0 `xml:"Streckenwechsel0"`
	Streckenwechsel1 []*Streckenwechsel1 `xml:"Streckenwechsel1"`
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
	StreckenNr []*Strecke        `xml:"Strecke"`
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
