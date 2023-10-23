#include "extended_ks.h"
#include "include/Input.h"

constinit auto script = NewScript().

	SetPlayerPos<Mario>(-1187, 254, 6167) (1).
	SetPlayerPos<Luigi>(-1487, 254, 6467) (1).
	SetPlayerPos<Wario>(-1287, 254, 6767) (1).
	SetPlayerPos<Yoshi>( -987, 254, 6567) (1).

	DisableAmbientSoundEffects() (1).

	ActivatePlayer<Mario>() (1).
	ActivatePlayer<Luigi>() (11).
	ActivatePlayer<Wario>() (21).
	ActivatePlayer<Yoshi>() (31).

	PlayerHoldButtons(Input::R) (89).
	PlayerHoldButtons(Input::A) (90).

	SetPlayerAngleY<Mario>(-180_deg) (60).
	SetPlayerAngleY<Luigi>(  83_deg) (70).
	SetPlayerAngleY<Wario>( 175_deg) (80).
	SetPlayerAngleY<Yoshi>(-102_deg) (90).

	LerpCamPos(-2000, 870, 6000, 2) (500, 530).
	LerpCamTarget(-3500, 250, 7000, 1) (560, 590).

	PlayLong<Mario>(3, 0x90) (100, 180).

	MovePlayer<Luigi>(10, 0, -20) (100, 180).
	TurnPlayer<Wario>(-5_deg)     (100, 172).
	TurnPlayer<Yoshi>(10_deg)     (100, 172).

	ExpDecayPlayerAngleY<Mario>(90_deg, 15) (200, 280).
	ExpDecayPlayerAngleY<Luigi>(90_deg, 15) (200, 280).
	ExpDecayPlayerAngleY<Wario>(90_deg, 15) (200, 280).
	ExpDecayPlayerAngleY<Yoshi>(90_deg, 15) (200, 280).

	ExpDecayPlayerAngleY<Mario>(-90_deg, 10) (280, 360).
	ExpDecayPlayerAngleY<Luigi>(-90_deg, 10) (280, 360).
	ExpDecayPlayerAngleY<Wario>(-90_deg, 10) (280, 360).
	ExpDecayPlayerAngleY<Yoshi>(-90_deg, 10) (280, 360).

	HurtPlayer  <Yoshi>({-1187, 254, 6167}, 2, 20._f) (360).
	BurnPlayer  <Wario>()                             (390).
	ShockPlayer <Mario>()                             (420).
	BouncePlayer<Luigi>(50._f)                        (450).
	HurtPlayer  <Luigi>({0, 254, 0}, 2, 100._f)       (458).
	BouncePlayer<Mario>(50._f)                        (470).

	Print("\nMario's position: ") (500). PrintPlayerPos<Mario>() (500).
	Print("\nLuigi's position: ") (510). PrintPlayerPos<Luigi>() (510).
	Print("\nWario's position: ") (520). PrintPlayerPos<Wario>() (520).
	Print("\nYoshi's position: ") (530). PrintPlayerPos<Yoshi>() (530).
	Print("\nCamera position:  ") (540). PrintCamPos          () (540).
	Print("\nCamera target:    ") (550). PrintCamTarget       () (550).
	Print("\nFrame counter:    ") (560). PrintFrameCounter    () (560).
	Print("\n") (560).

	EnableAmbientSoundEffects() (600).

	Print("\nDone!\n") (630).

	End();

void init()
{
	script.Run();
}

void cleanup() {}
