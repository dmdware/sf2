













#include "language.h"
#include "platform.h"
#include "gui/richtext.h"
#include "utils.h"

RichText STRTABLE[STR_TYPES];
std::string g_lang = "eng";

const char* STRFILE[STR_TYPES] =
{
	"000 self lang.txt",
	"001 idcode.txt",
	"002 newgame.txt",
	"003 loadgame.txt",
	"004 joingame.txt",
	"005 hostgame.txt",
	"006 options.txt",
	"007 quit.txt",
	"008 windowed.txt",
	"009 fullscreen.txt",
	"010 done.txt",
	"011 tutorial.txt",
	"012 title.txt",
	"013 maped.txt",
	"014 volumepct.txt",
	"015 lanonlynotpub.txt",
	"016 nomatchsvnot.txt",
	"017 conmatchsvlist.txt",
	"018 conmatchlisting.txt",
	"019 contomatch.txt",
	"020 conmatchgetnext.txt",
	"021 conmatchreqsv.txt",
	"022 getsvlist.txt",
	"023 add.txt",
	"024 clear.txt",
	"025 qref.txt",
	"026 refresh.txt",
	"027 lanref.txt",
	"028 address.txt",
	"029 port.txt",
	"030 unit labourer.txt",
	"031 unit battlecomp.txt",
	"032 unit truck.txt",
	"033 unit carlyle.txt",
	"034 bl house1.txt",
	"035 bl store.txt",
	"036 bl trfac.txt",
	"037 bl farm.txt",
	"038 bl shmine.txt",
	"039 bl ironsm.txt",
	"040 bl oilwell.txt",
	"041 bl oilref.txt",
	"042 bl nucpow.txt",
	"043 bl coalpow.txt",
	"044 bl cempl.txt",
	"045 bl chempl.txt",
	"046 bl elecpl.txt",
	"047 bl gasstn.txt",
	"048 cd road.txt",
	"049 cd powl.txt",
	"050 cd crpipe.txt",
	"051 lowerresrec.txt",
	"052 house desc.txt",
	"053 fac desc.txt",
	"054 oilref desc.txt",
	"055 coalpow desc.txt",
	"056 chempl desc.txt",
	"057 gasstn desc.txt",
	"058 cempl desc.txt",
	"059 ironsm desc.txt",
	"060 nucpow desc.txt",
	"061 farm desc.txt",
	"062 store desc.txt",
	"063 oilwell desc.txt",
	"064 shmine desc.txt",
	"065 road desc.txt",
	"066 powl desc.txt",
	"067 crpipe desc.txt",
	"068 res funds.txt",
	"069 res labour.txt",
	"070 res housing.txt",
	"071 res farmprods.txt",
	"072 res prod.txt",
	"073 res retfood.txt",
	"074 res croil.txt",
	"075 res wsfuel.txt",
	"076 res retfuel.txt",
	"077 res energy.txt",
	"078 res chems.txt",
	"079 res ironore.txt",
	"080 res metal.txt",
	"081 res stone.txt",
	"082 res cement.txt",
	"083 res coal.txt",
	"084 res uran.txt",
	"085 meas dollars.txt",
	"086 meas labsecs.txt",
	"087 meas occ.txt",
	"088 meas grams.txt",
	"089 meas gallons.txt",
	"090 meas kwatts.txt",
	"091 meas liters.txt",
	"092 meas kgrams.txt",
	"093 meas bushels.txt",
	"094 addsv desc.txt",
	"095 clearsv desc.txt",
	"096 refsv desc.txt",
	"097 getsv desc.txt",
	"098 srlan desc.txt",
	"099 earnprod.txt",
	"100 expcon.txt",
	"101 close.txt",
	"102 version.txt",
	"103 newmap.txt",
	"104 elevation.txt",
	"105 terrain.txt",
	"106 foliage.txt",
	"107 mv.txt",
	"108 bl.txt",
	"109 conduits.txt",
	"110 resources.txt",
	"111 triggers.txt",
	"112 place.txt",
	"113 delete.txt",
	"114 raise.txt",
	"115 lower.txt",
	"116 spread.txt",
	"117 loadmap.txt",
	"118 savemap.txt",
	"119 qsave.txt",
	"120 save.txt",
	"121 savetofile.txt",
	"122 delthesave.txt",
	"123 showdebug.txt",
	"124 showtransx.txt",
	"125 savegame.txt",
	"126 loadgame.txt",
	"127 pauseupd.txt",
	"128 playunlock.txt",
	"129 fastunlock.txt",
	"130 truckmgr.txt",
	"131 quittomenu.txt",
	"132 cancel.txt",
	"133 continue.txt",
	"134 set.txt",
	"135 graphs.txt",
	"136 stocklist.txt",
	"137 none.txt",
	"138 buy.txt",
	"139 prodlevel.txt",
	"140 wage.txt",
	"141 priceof.txt",
	"142 cost.txt",
	"143 need.txt",
	"144 have.txt",
	"145 toordera.txt",
	"146 insufficient.txt",
	"147 atmanuf.txt",
	"148 toproduce.txt",
	"149 manufda.txt",
	"150 for.txt",
	"151 ordereda.txt",
	"152 from.txt",
	"153 setprice.txt",
	"154 at.txt",
	"155 to.txt",
	"156 setwage.txt",
	"157 setconwage.txt",
	"158 settransppr.txt",
	"159 setdrwage.txt",
	"160 setprodlev.txt",
	"161 setmanpr.txt",
	"162 of.txt",
	"163 join.txt",
	"164 joincurgame.txt",
	"165 noconmatch.txt",
	"166 conmatch.txt",
	"167 nosv.txt",
	"168 joining.txt",
	"169 copyright.txt",
	"170 concomp.txt",
	"171 joinedgame.txt",
	"172 leftgame.txt",
	"173 hostdisc.txt",
	"174 placeda.txt",
	"175 paused.txt",
	"176 pressedplay.txt",
	"177 pressedfast.txt",
	"178 ergamest.txt",
	"179 ermapmatch.txt",
	"180 erloadmap.txt",
	"181 ersvfull.txt",
	"182 ervermatch.txt",
	"183 timedout.txt",
	"184 waskicked.txt",
	"185 second.txt",
	"186 player.txt",
	"187 networth.txt",
	"188 utilprov.txt",
	"189 thruput.txt",
	"190 transpcost.txt",
	"191 drwage.txt",
	"192 load.txt",
	"193 loadselgame.txt",
	"194 delselgame.txt",
	"195 makepub.txt",
	"196 create.txt",
	"197 gameroom.txt",
	"198 gamename.txt",
	"199 map.txt",
	"200 conreqs.txt",
	"201 inputs.txt",
	"202 outputs.txt",
	"203 eviction.txt",
	"204 starvation.txt",
	"205 population.txt",
	"206 growth.txt",
	"207 conwage.txt",
	"208 ready.txt",
	"209 quit.txt",
	"210 toggleready.txt",
	"211 exitsess.txt",
	"212 tilewx.txt",
	"213 tilewy.txt",
	"214 random.txt",
	"215 randseed.txt",
	"216 generate.txt",
	"217 startingin.txt",
	"218 rest.txt",
	"219 idle.txt",
	"220 tojob.txt",
	"221 atjob.txt",
	"222 cstr.txt",
	"223 trucking.txt",
	"224 tosup.txt",
	"225 todem.txt",
	"226 toref.txt",
	"227 refuel.txt",
	"228 offload.txt",
	"229 loadup.txt",
	"230 tostore.txt",
	"231 atstore.txt",
	"232 tohome.txt",
	"233 athome.txt",
	"234 escape.txt",
	"235 ranks.txt",
	"236 borders.txt",
	"237 bought.txt",
	"238 putup.txt",
	"239 forsalefor.txt",
	"240 tookdown.txt",
	"241 fromsale.txt",
	"242 changedproppriceof.txt",
	"243 propertyprice.txt",
	"244 buyprop.txt",
	"245 buythisbl.txt",
	"246 forsale.txt",
	"247 donthavetr.txt",
	"248 playsloton.txt",
	"249 isaiplayer.txt",
	"250 parentstate.txt",
	"251 insttype.txt",
	"252 mustbesingle.txt",
	"253 stilldown.txt",
	"254 mapnotloaded.txt",
	"255 errloadm.txt",
	"256 timedout.txt",
	"257 left.txt",
	"258 overmap.txt",
	"259 insufbuy.txt",
	"260 widthmustbe.txt",
	"261 and.txt",
	"262 inout.txt",
	"263 dem.txt",
	"264 dataunav.txt",
	"265 demsupavc.txt",
	"266 sup.txt",
	"267 avc.txt",
	"268 demol.txt",
	"269 demold.txt",
	"270 mgc.txt",
	"271 placed.txt",
	"272 avpe.txt",
	"273 avpf.txt",
	"274 totpe.txt",
	"275 totpf.txt",
	"276 bank.txt",
	"277 bankdesc.txt",
	"278 cvnote.txt",
	"279 meas prod.txt",
	"280 t.txt",
	"281 t0.txt",
	"282 t1.txt",
	"283 t2.txt",
	"284 t3.txt",
	"285 t4.txt",
	"286 t5.txt",
	"287 t6.txt",
	"288 t7.txt",
	"289 t8.txt",
	"290 t9.txt",
	"291 t10.txt",
	"292 t11.txt",
	"293 t12.txt",
	"294 t13.txt",
	"295 t14.txt",
	"296 t15.txt",
	"297 t16.txt",
	"298 t17.txt",
	"299 t18.txt",
	"300 t19.txt",
	"301 t20.txt",
	"302 t21.txt",
	"303 t22.txt",
	"304 t23.txt",
	"305 t24.txt",
	"306 t25.txt",
	"307 t26.txt",
	"308 t27.txt",
	"309 t28.txt",
	"310 t29.txt",
	"311 t30.txt"
};

std::string LoadLangStr(const char* lang, const char* file)
{
	char relative[128];
	sprintf(relative, "lang/%s/%s", lang, file);
	char fullpath[SFH_MAX_PATH+1];
	FullPath(relative, fullpath);

	return LoadTextFile(fullpath).c_str();
}

ecbool SwitchLang(const char* dir)
{
	char relative[128];
	sprintf(relative, "lang/%s/%s", dir, STRFILE[0]);
	char fullpath[SFH_MAX_PATH+1];
	FullPath(relative, fullpath);

	FILE* fp = fopen(fullpath, "r");

	if(!fp)
		return ecfalse;

	fclose(fp);

	g_lang = dir;

	for(int i=0; i<STR_TYPES; i++)
	{
		STRTABLE[i] = RichText(LoadLangStr(dir, STRFILE[i]).c_str());
	}

	return ectrue;
}

#if 0
=
{
	{	//0 STR_SELFLANG
		"Bulgarian / Български",	//LANG_BUL		0
		"Chinese, Simp. / 简体中国",	//LANG_CHS		1
		"Chinese, Trad. / 中国传统",	//LANG_CHT		2
		"English / English",	//LANG_ENG		3
		"French / Français",	//LANG_FRE		4
		"German / Deutsch",	//LANG_GER		5
		"Italian / Italiano",	//LANG_ITL		6
		"Japanese / 日本人",	//LANG_JAP		7
		"Korean / 한국어",	//LANG_KOR		8
		"Polish / Polski",	//LANG_POL		9
		"Portuguese / Português",	//LANG_POR		10
		"Russian / Русский",	//LANG_RUS		11
		"Spanish / Español"	//LANG_SPA		12
	},
	{	// 1 STR_IDCODE
		"bul",
		"chs",
		"cht",
		"eng",
		"fre",
		"ger",
		"itl",
		"jap",
		"kor",
		"pol",
		"por",
		"rus",
		"spa"
	},
	{	//STR_NEWGAME			2
		"Започнете нова игра",	//LANG_BUL		0
		"开始新游戏",	//LANG_CHS		1
		"開始新遊戲",	//LANG_CHT		2
		"Start a new game",	//LANG_ENG		3
		"Commencez une nouvelle partie",	//LANG_FRE		4
		"Startet ein neues Spiel",	//LANG_GER		5
		"Inizia una nuova partita",	//LANG_ITL		6
		"新しいゲームを開始します",	//LANG_JAP		7
		"새로운 게임을 시작합니다",	//LANG_KOR		8
		"Rozpocznij nową grę",	//LANG_POL		9
		"Iniciar um novo jogo",	//LANG_POR		10
		"Начать новую игру",	//LANG_RUS		11
		"Spanish / Español"	//LANG_SPA		12
	},
	
	{	//STR_LOADGAME		3
		"Заредете запазена игра",	//LANG_BUL		0
		"加载保存的游戏",	//LANG_CHS		1
		"加載保存的遊戲",	//LANG_CHT		2
		"Load a saved game",	//LANG_ENG		3
		"Charger une partie sauvegardée",	//LANG_FRE		4
		"Laden Sie ein gespeichertes Spiel",	//LANG_GER		5
		"Caricare un gioco salvato",	//LANG_ITL		6
		"保存したゲームをロード",	//LANG_JAP		7
		"저장된 게임을로드",	//LANG_KOR		8
		"Wczytać zapisanej gry",	//LANG_POL		9
		"Carregar um jogo salvo",	//LANG_POR		10
		"Загрузите сохраненную игру",	//LANG_RUS		11
		"Cargar una partida guardada"	//LANG_SPA		12
	},
	
	{	//STR_JOINGAME		4
		"Присъединете се към мултиплейър игра",	//LANG_BUL		0
		"加入多人游戏",	//LANG_CHS		1
		"加入多人遊戲",	//LANG_CHT		2
		"Join a multiplayer game",	//LANG_ENG		3
		"Joignez-vous à un jeu multijoueur",	//LANG_FRE		4
		"Nehmen Sie an einem Mehrspieler-Spiel",	//LANG_GER		5
		"Partecipa a un gioco multiplayer",	//LANG_ITL		6
		"マルチプレイヤーゲームに参加",	//LANG_JAP		7
		"멀티 플레이어 게임에 참여",	//LANG_KOR		8
		"Dołącz do gry wieloosobowej",	//LANG_POL		9
		"Junte-se a um jogo multiplayer",	//LANG_POR		10
		"Присоединиться к многопользовательской игре",	//LANG_RUS		11
		"Únete a una partida multijugador"	//LANG_SPA		12
	},
	{	//STR_HOSTGAME		5
		"Домакин на мултиплейър игра",	//LANG_BUL		0
		"主机多人游戏",	//LANG_CHS		1
		"主機多人遊戲",	//LANG_CHT		2
		"Host a multiplayer game",	//LANG_ENG		3
		"Héberger une partie multijoueur",	//LANG_FRE		4
		"Host ein Multiplayer-Spiel",	//LANG_GER		5
		"Ospitare un gioco multiplayer",	//LANG_ITL		6
		"マルチプレイヤーゲームをホストします",	//LANG_JAP		7
		"멀티 플레이 게임을 호스트",	//LANG_KOR		8
		"Host grę wieloosobową",	//LANG_POL		9
		"Hospedar um jogo multiplayer",	//LANG_POR		10
		"Принимающая многопользовательскую игру",	//LANG_RUS		11
		"Organiza una partida multijugador"	//LANG_SPA		12
	},
	{	//STR_OPTIONS			6
		"Регулирайте опции",	//LANG_BUL		0
		"调整选项",	//LANG_CHS		1
		"調整選項",	//LANG_CHT		2
		"Adjust options",	//LANG_ENG		3
		"Réglez les options",	//LANG_FRE		4
		"Passen Sie Optionen",	//LANG_GER		5
		"Regolare le opzioni",	//LANG_ITL		6
		"オプションを調整します",	//LANG_JAP		7
		"옵션을 조정합니다",	//LANG_KOR		8
		"Dostosuj ustawienia",	//LANG_POL		9
		"Ajuste as opções",	//LANG_POR		10
		"Отрегулируйте параметры",	//LANG_RUS		11
		"Ajuste las opciones"	//LANG_SPA		12
	},
	{	//STR_QUIT			7
		"Спри, за да десктоп",	//LANG_BUL		0
		"退出到桌面",	//LANG_CHS		1
		"退出到桌面",	//LANG_CHT		2
		"Quit to desktop",	//LANG_ENG		3
		"Quittez le bureau",	//LANG_FRE		4
		"Beenden Sie auf dem Desktop",	//LANG_GER		5
		"Esci per desktop",	//LANG_ITL		6
		"デスクトップに終了します",	//LANG_JAP		7
		"바탕 화면에 종료합니다",	//LANG_KOR		8
		"Wyjdź do pulpitu",	//LANG_POL		9
		"Sair para desktop",	//LANG_POR		10
		"Выход на рабочий стол",	//LANG_RUS		11
		"Salir para escritorio"	//LANG_SPA		12
	},
	{	//STR_WINDOWED		8
		"В прозорец",	//LANG_BUL		0
		"窗",	//LANG_CHS		1
		"窗",	//LANG_CHT		2
		"Windowed",	//LANG_ENG		3
		"Fenêtré",	//LANG_FRE		4
		"In-Fenster",	//LANG_GER		5
		"Nella finestra",	//LANG_ITL		6
		"ウィンドウで",	//LANG_JAP		7
		"창에서",	//LANG_KOR		8
		"W oknie",	//LANG_POL		9
		"Na janela",	//LANG_POR		10
		"В окне",	//LANG_RUS		11
		"En la ventana"	//LANG_SPA		12
	},
	{	//STR_FULLSCREEN		9
		"Цял Екран",	//LANG_BUL		0
		"全屏",	//LANG_CHS		1
		"全屏",	//LANG_CHT		2
		"Fullscreen",	//LANG_ENG		3
		"Plein Écran",	//LANG_FRE		4
		"Vollbild",	//LANG_GER		5
		"Schermo Intero",	//LANG_ITL		6
		"全画面表示",	//LANG_JAP		7
		"전체 화면",	//LANG_KOR		8
		"Pełny Ekran",	//LANG_POL		9
		"Tela Cheia",	//LANG_POR		10
		"Полноэкранный",	//LANG_RUS		11
		"Pantalla Completa"	//LANG_SPA		12
	},
	{	//STR_DONE			10
		"Съставено",	//LANG_BUL		0
		"完成",	//LANG_CHS		1
		"完成",	//LANG_CHT		2
		"Done",	//LANG_ENG		3
		"Terminé",	//LANG_FRE		4
		"Fertig",	//LANG_GER		5
		"Fatto",	//LANG_ITL		6
		"完了",	//LANG_JAP		7
		"완료",	//LANG_KOR		8
		"Gotowe",	//LANG_POL		9
		"Feito",	//LANG_POR		10
		"Готово",	//LANG_RUS		11
		"Hecho"	//LANG_SPA		12
	},
	{	//STR_TUTORIAL		11
		"Играйте начинаещи",	//LANG_BUL		0
		"播放教程",	//LANG_CHS		1
		"播放教程",	//LANG_CHT		2
		"Play the tutorial",	//LANG_ENG		3
		"Jouer le tutoriel",	//LANG_FRE		4
		"Spielen Sie das Tutorial",	//LANG_GER		5
		"Gioca il tutorial",	//LANG_ITL		6
		"チュートリアルをプレイ",	//LANG_JAP		7
		"자습서를 재생합니다",	//LANG_KOR		8
		"Zagraj samouczek",	//LANG_POL		9
		"Jogue o tutorial",	//LANG_POR		10
		"Играть учебник",	//LANG_RUS		11
		"Juega el tutorial"	//LANG_SPA		12
	},
	{	//STR_TITLE		12
		"Членки, Фирми и Домакинства",	//LANG_BUL		0
		"国家，企业，家庭及",	//LANG_CHS		1
		"國家，企業，家庭及",	//LANG_CHT		2
		"States, Firms, and Households",	//LANG_ENG		3
		"Unis, les Entreprises, et les Ménages",	//LANG_FRE		4
		"Staaten, Firmen, und Haushalte",	//LANG_GER		5
		"Stati, Imprese, e Famiglie",	//LANG_ITL		6
		"国、企業、および世帯",	//LANG_JAP		7
		"미국, 기업, 가구",	//LANG_KOR		8
		"Członkowskie, Firmy, i Gospodarstw Domowych",	//LANG_POL		9
		"Membros, as Empresas, e Residências",	//LANG_POR		10
		"Государства, Фирмы, и Домохозяйства",	//LANG_RUS		11
		"Estados, Empresas, y Hogares"	//LANG_SPA		12
	},
	
	{	//STR_DOLLARS			13
		"Bulgarian / Български",	//LANG_BUL		0
		"Chinese, Simp. / 简体中国",	//LANG_CHS		1
		"Chinese, Trad. / 中国传统",	//LANG_CHT		2
		"English / English",	//LANG_ENG		3
		"French / Français",	//LANG_FRE		4
		"German / Deutsch",	//LANG_GER		5
		"Italian / Italiano",	//LANG_ITL		6
		"Japanese / 日本人",	//LANG_JAP		7
		"Korean / 한국어",	//LANG_KOR		8
		"Polish / Polski",	//LANG_POL		9
		"Portuguese / Português",	//LANG_POR		10
		"Russian / Русский",	//LANG_RUS		11
		"Spanish / Español"	//LANG_SPA		12
	},
	{	//STR_LABOUR			14
		"Bulgarian / Български",	//LANG_BUL		0
		"Chinese, Simp. / 简体中国",	//LANG_CHS		1
		"Chinese, Trad. / 中国传统",	//LANG_CHT		2
		"English / English",	//LANG_ENG		3
		"French / Français",	//LANG_FRE		4
		"German / Deutsch",	//LANG_GER		5
		"Italian / Italiano",	//LANG_ITL		6
		"Japanese / 日本人",	//LANG_JAP		7
		"Korean / 한국어",	//LANG_KOR		8
		"Polish / Polski",	//LANG_POL		9
		"Portuguese / Português",	//LANG_POR		10
		"Russian / Русский",	//LANG_RUS		11
		"Spanish / Español"	//LANG_SPA		12
	},
	{	//STR_HOUSING			15
		"Bulgarian / Български",	//LANG_BUL		0
		"Chinese, Simp. / 简体中国",	//LANG_CHS		1
		"Chinese, Trad. / 中国传统",	//LANG_CHT		2
		"English / English",	//LANG_ENG		3
		"French / Français",	//LANG_FRE		4
		"German / Deutsch",	//LANG_GER		5
		"Italian / Italiano",	//LANG_ITL		6
		"Japanese / 日本人",	//LANG_JAP		7
		"Korean / 한국어",	//LANG_KOR		8
		"Polish / Polski",	//LANG_POL		9
		"Portuguese / Português",	//LANG_POR		10
		"Russian / Русский",	//LANG_RUS		11
		"Spanish / Español"	//LANG_SPA		12
	},
	{	//STR_FARMPRODUCTS	16
		"Bulgarian / Български",	//LANG_BUL		0
		"Chinese, Simp. / 简体中国",	//LANG_CHS		1
		"Chinese, Trad. / 中国传统",	//LANG_CHT		2
		"English / English",	//LANG_ENG		3
		"French / Français",	//LANG_FRE		4
		"German / Deutsch",	//LANG_GER		5
		"Italian / Italiano",	//LANG_ITL		6
		"Japanese / 日本人",	//LANG_JAP		7
		"Korean / 한국어",	//LANG_KOR		8
		"Polish / Polski",	//LANG_POL		9
		"Portuguese / Português",	//LANG_POR		10
		"Russian / Русский",	//LANG_RUS		11
		"Spanish / Español"	//LANG_SPA		12
	},
	{	//STR_RETFOOD			17
		"Bulgarian / Български",	//LANG_BUL		0
		"Chinese, Simp. / 简体中国",	//LANG_CHS		1
		"Chinese, Trad. / 中国传统",	//LANG_CHT		2
		"English / English",	//LANG_ENG		3
		"French / Français",	//LANG_FRE		4
		"German / Deutsch",	//LANG_GER		5
		"Italian / Italiano",	//LANG_ITL		6
		"Japanese / 日本人",	//LANG_JAP		7
		"Korean / 한국어",	//LANG_KOR		8
		"Polish / Polski",	//LANG_POL		9
		"Portuguese / Português",	//LANG_POR		10
		"Russian / Русский",	//LANG_RUS		11
		"Spanish / Español"	//LANG_SPA		12
	},
	{	//STR_CHEMICALS		18
		"Bulgarian / Български",	//LANG_BUL		0
		"Chinese, Simp. / 简体中国",	//LANG_CHS		1
		"Chinese, Trad. / 中国传统",	//LANG_CHT		2
		"English / English",	//LANG_ENG		3
		"French / Français",	//LANG_FRE		4
		"German / Deutsch",	//LANG_GER		5
		"Italian / Italiano",	//LANG_ITL		6
		"Japanese / 日本人",	//LANG_JAP		7
		"Korean / 한국어",	//LANG_KOR		8
		"Polish / Polski",	//LANG_POL		9
		"Portuguese / Português",	//LANG_POR		10
		"Russian / Русский",	//LANG_RUS		11
		"Spanish / Español"	//LANG_SPA		12
	},
	{	//STR_IRONORE			19
		"Bulgarian / Български",	//LANG_BUL		0
		"Chinese, Simp. / 简体中国",	//LANG_CHS		1
		"Chinese, Trad. / 中国传统",	//LANG_CHT		2
		"English / English",	//LANG_ENG		3
		"French / Français",	//LANG_FRE		4
		"German / Deutsch",	//LANG_GER		5
		"Italian / Italiano",	//LANG_ITL		6
		"Japanese / 日本人",	//LANG_JAP		7
		"Korean / 한국어",	//LANG_KOR		8
		"Polish / Polski",	//LANG_POL		9
		"Portuguese / Português",	//LANG_POR		10
		"Russian / Русский",	//LANG_RUS		11
		"Spanish / Español"	//LANG_SPA		12
	},
	{	//STR_IRONORE			19
		"Bulgarian / Български",	//LANG_BUL		0
		"Chinese, Simp. / 简体中国",	//LANG_CHS		1
		"Chinese, Trad. / 中国传统",	//LANG_CHT		2
		"English / English",	//LANG_ENG		3
		"French / Français",	//LANG_FRE		4
		"German / Deutsch",	//LANG_GER		5
		"Italian / Italiano",	//LANG_ITL		6
		"Japanese / 日本人",	//LANG_JAP		7
		"Korean / 한국어",	//LANG_KOR		8
		"Polish / Polski",	//LANG_POL		9
		"Portuguese / Português",	//LANG_POR		10
		"Russian / Русский",	//LANG_RUS		11
		"Spanish / Español"	//LANG_SPA		12
	},
	{	//STR_METAL			20
		"Bulgarian / Български",	//LANG_BUL		0
		"Chinese, Simp. / 简体中国",	//LANG_CHS		1
		"Chinese, Trad. / 中国传统",	//LANG_CHT		2
		"English / English",	//LANG_ENG		3
		"French / Français",	//LANG_FRE		4
		"German / Deutsch",	//LANG_GER		5
		"Italian / Italiano",	//LANG_ITL		6
		"Japanese / 日本人",	//LANG_JAP		7
		"Korean / 한국어",	//LANG_KOR		8
		"Polish / Polski",	//LANG_POL		9
		"Portuguese / Português",	//LANG_POR		10
		"Russian / Русский",	//LANG_RUS		11
		"Spanish / Español"	//LANG_SPA		12
	},
	{	//STR_STONE			21
		"Bulgarian / Български",	//LANG_BUL		0
		"Chinese, Simp. / 简体中国",	//LANG_CHS		1
		"Chinese, Trad. / 中国传统",	//LANG_CHT		2
		"English / English",	//LANG_ENG		3
		"French / Français",	//LANG_FRE		4
		"German / Deutsch",	//LANG_GER		5
		"Italian / Italiano",	//LANG_ITL		6
		"Japanese / 日本人",	//LANG_JAP		7
		"Korean / 한국어",	//LANG_KOR		8
		"Polish / Polski",	//LANG_POL		9
		"Portuguese / Português",	//LANG_POR		10
		"Russian / Русский",	//LANG_RUS		11
		"Spanish / Español"	//LANG_SPA		12
	},
	{	//STR_CEMENT			22
		"Bulgarian / Български",	//LANG_BUL		0
		"Chinese, Simp. / 简体中国",	//LANG_CHS		1
		"Chinese, Trad. / 中国传统",	//LANG_CHT		2
		"English / English",	//LANG_ENG		3
		"French / Français",	//LANG_FRE		4
		"German / Deutsch",	//LANG_GER		5
		"Italian / Italiano",	//LANG_ITL		6
		"Japanese / 日本人",	//LANG_JAP		7
		"Korean / 한국어",	//LANG_KOR		8
		"Polish / Polski",	//LANG_POL		9
		"Portuguese / Português",	//LANG_POR		10
		"Russian / Русский",	//LANG_RUS		11
		"Spanish / Español"	//LANG_SPA		12
	},
	{	//STR_COAL			23
		"Bulgarian / Български",	//LANG_BUL		0
		"Chinese, Simp. / 简体中国",	//LANG_CHS		1
		"Chinese, Trad. / 中国传统",	//LANG_CHT		2
		"English / English",	//LANG_ENG		3
		"French / Français",	//LANG_FRE		4
		"German / Deutsch",	//LANG_GER		5
		"Italian / Italiano",	//LANG_ITL		6
		"Japanese / 日本人",	//LANG_JAP		7
		"Korean / 한국어",	//LANG_KOR		8
		"Polish / Polski",	//LANG_POL		9
		"Portuguese / Português",	//LANG_POR		10
		"Russian / Русский",	//LANG_RUS		11
		"Spanish / Español"	//LANG_SPA		12
	},
	{	//STR_URANIUM			24
		"Bulgarian / Български",	//LANG_BUL		0
		"Chinese, Simp. / 简体中国",	//LANG_CHS		1
		"Chinese, Trad. / 中国传统",	//LANG_CHT		2
		"English / English",	//LANG_ENG		3
		"French / Français",	//LANG_FRE		4
		"German / Deutsch",	//LANG_GER		5
		"Italian / Italiano",	//LANG_ITL		6
		"Japanese / 日本人",	//LANG_JAP		7
		"Korean / 한국어",	//LANG_KOR		8
		"Polish / Polski",	//LANG_POL		9
		"Portuguese / Português",	//LANG_POR		10
		"Russian / Русский",	//LANG_RUS		11
		"Spanish / Español"	//LANG_SPA		12
	},
	{	//STR_PRODUCTION		25
		"Bulgarian / Български",	//LANG_BUL		0
		"Chinese, Simp. / 简体中国",	//LANG_CHS		1
		"Chinese, Trad. / 中国传统",	//LANG_CHT		2
		"English / English",	//LANG_ENG		3
		"French / Français",	//LANG_FRE		4
		"German / Deutsch",	//LANG_GER		5
		"Italian / Italiano",	//LANG_ITL		6
		"Japanese / 日本人",	//LANG_JAP		7
		"Korean / 한국어",	//LANG_KOR		8
		"Polish / Polski",	//LANG_POL		9
		"Portuguese / Português",	//LANG_POR		10
		"Russian / Русский",	//LANG_RUS		11
		"Spanish / Español"	//LANG_SPA		12
	},
	{	//STR_CRUDEOIL		26
		"Bulgarian / Български",	//LANG_BUL		0
		"Chinese, Simp. / 简体中国",	//LANG_CHS		1
		"Chinese, Trad. / 中国传统",	//LANG_CHT		2
		"English / English",	//LANG_ENG		3
		"French / Français",	//LANG_FRE		4
		"German / Deutsch",	//LANG_GER		5
		"Italian / Italiano",	//LANG_ITL		6
		"Japanese / 日本人",	//LANG_JAP		7
		"Korean / 한국어",	//LANG_KOR		8
		"Polish / Polski",	//LANG_POL		9
		"Portuguese / Português",	//LANG_POR		10
		"Russian / Русский",	//LANG_RUS		11
		"Spanish / Español"	//LANG_SPA		12
	},
	{	//STR_WSFUEL			27
		"Bulgarian / Български",	//LANG_BUL		0
		"Chinese, Simp. / 简体中国",	//LANG_CHS		1
		"Chinese, Trad. / 中国传统",	//LANG_CHT		2
		"English / English",	//LANG_ENG		3
		"French / Français",	//LANG_FRE		4
		"German / Deutsch",	//LANG_GER		5
		"Italian / Italiano",	//LANG_ITL		6
		"Japanese / 日本人",	//LANG_JAP		7
		"Korean / 한국어",	//LANG_KOR		8
		"Polish / Polski",	//LANG_POL		9
		"Portuguese / Português",	//LANG_POR		10
		"Russian / Русский",	//LANG_RUS		11
		"Spanish / Español"	//LANG_SPA		12
	},
	{	//STR_RETFUEL			28
		"Bulgarian / Български",	//LANG_BUL		0
		"Chinese, Simp. / 简体中国",	//LANG_CHS		1
		"Chinese, Trad. / 中国传统",	//LANG_CHT		2
		"English / English",	//LANG_ENG		3
		"French / Français",	//LANG_FRE		4
		"German / Deutsch",	//LANG_GER		5
		"Italian / Italiano",	//LANG_ITL		6
		"Japanese / 日本人",	//LANG_JAP		7
		"Korean / 한국어",	//LANG_KOR		8
		"Polish / Polski",	//LANG_POL		9
		"Portuguese / Português",	//LANG_POR		10
		"Russian / Русский",	//LANG_RUS		11
		"Spanish / Español"	//LANG_SPA		12
	},
	{	//STR_ENERGY			29
		"Bulgarian / Български",	//LANG_BUL		0
		"Chinese, Simp. / 简体中国",	//LANG_CHS		1
		"Chinese, Trad. / 中国传统",	//LANG_CHT		2
		"English / English",	//LANG_ENG		3
		"French / Français",	//LANG_FRE		4
		"German / Deutsch",	//LANG_GER		5
		"Italian / Italiano",	//LANG_ITL		6
		"Japanese / 日本人",	//LANG_JAP		7
		"Korean / 한국어",	//LANG_KOR		8
		"Polish / Polski",	//LANG_POL		9
		"Portuguese / Português",	//LANG_POR		10
		"Russian / Русский",	//LANG_RUS		11
		"Spanish / Español"	//LANG_SPA		12
	},
	{	//STR_LABOURER		30
		"Bulgarian / Български",	//LANG_BUL		0
		"Chinese, Simp. / 简体中国",	//LANG_CHS		1
		"Chinese, Trad. / 中国传统",	//LANG_CHT		2
		"English / English",	//LANG_ENG		3
		"French / Français",	//LANG_FRE		4
		"German / Deutsch",	//LANG_GER		5
		"Italian / Italiano",	//LANG_ITL		6
		"Japanese / 日本人",	//LANG_JAP		7
		"Korean / 한국어",	//LANG_KOR		8
		"Polish / Polski",	//LANG_POL		9
		"Portuguese / Português",	//LANG_POR		10
		"Russian / Русский",	//LANG_RUS		11
		"Spanish / Español"	//LANG_SPA		12
	},
	{	//STR_BATTLECOMP		31
		"Bulgarian / Български",	//LANG_BUL		0
		"Chinese, Simp. / 简体中国",	//LANG_CHS		1
		"Chinese, Trad. / 中国传统",	//LANG_CHT		2
		"English / English",	//LANG_ENG		3
		"French / Français",	//LANG_FRE		4
		"German / Deutsch",	//LANG_GER		5
		"Italian / Italiano",	//LANG_ITL		6
		"Japanese / 日本人",	//LANG_JAP		7
		"Korean / 한국어",	//LANG_KOR		8
		"Polish / Polski",	//LANG_POL		9
		"Portuguese / Português",	//LANG_POR		10
		"Russian / Русский",	//LANG_RUS		11
		"Spanish / Español"	//LANG_SPA		12
	},
	{	//STR_TRUCK			32
		"Bulgarian / Български",	//LANG_BUL		0
		"Chinese, Simp. / 简体中国",	//LANG_CHS		1
		"Chinese, Trad. / 中国传统",	//LANG_CHT		2
		"English / English",	//LANG_ENG		3
		"French / Français",	//LANG_FRE		4
		"German / Deutsch",	//LANG_GER		5
		"Italian / Italiano",	//LANG_ITL		6
		"Japanese / 日本人",	//LANG_JAP		7
		"Korean / 한국어",	//LANG_KOR		8
		"Polish / Polski",	//LANG_POL		9
		"Portuguese / Português",	//LANG_POR		10
		"Russian / Русский",	//LANG_RUS		11
		"Spanish / Español"	//LANG_SPA		12
	},
	{	//STR_CARLYLE			33
		"Bulgarian / Български",	//LANG_BUL		0
		"Chinese, Simp. / 简体中国",	//LANG_CHS		1
		"Chinese, Trad. / 中国传统",	//LANG_CHT		2
		"English / English",	//LANG_ENG		3
		"French / Français",	//LANG_FRE		4
		"German / Deutsch",	//LANG_GER		5
		"Italian / Italiano",	//LANG_ITL		6
		"Japanese / 日本人",	//LANG_JAP		7
		"Korean / 한국어",	//LANG_KOR		8
		"Polish / Polski",	//LANG_POL		9
		"Portuguese / Português",	//LANG_POR		10
		"Russian / Русский",	//LANG_RUS		11
		"Spanish / Español"	//LANG_SPA		12
	},
	{	//STR_HOUSE1			34
		"Bulgarian / Български",	//LANG_BUL		0
		"Chinese, Simp. / 简体中国",	//LANG_CHS		1
		"Chinese, Trad. / 中国传统",	//LANG_CHT		2
		"English / English",	//LANG_ENG		3
		"French / Français",	//LANG_FRE		4
		"German / Deutsch",	//LANG_GER		5
		"Italian / Italiano",	//LANG_ITL		6
		"Japanese / 日本人",	//LANG_JAP		7
		"Korean / 한국어",	//LANG_KOR		8
		"Polish / Polski",	//LANG_POL		9
		"Portuguese / Português",	//LANG_POR		10
		"Russian / Русский",	//LANG_RUS		11
		"Spanish / Español"	//LANG_SPA		12
	},
	{	//STR_STORE			35
		"Bulgarian / Български",	//LANG_BUL		0
		"Chinese, Simp. / 简体中国",	//LANG_CHS		1
		"Chinese, Trad. / 中国传统",	//LANG_CHT		2
		"English / English",	//LANG_ENG		3
		"French / Français",	//LANG_FRE		4
		"German / Deutsch",	//LANG_GER		5
		"Italian / Italiano",	//LANG_ITL		6
		"Japanese / 日本人",	//LANG_JAP		7
		"Korean / 한국어",	//LANG_KOR		8
		"Polish / Polski",	//LANG_POL		9
		"Portuguese / Português",	//LANG_POR		10
		"Russian / Русский",	//LANG_RUS		11
		"Spanish / Español"	//LANG_SPA		12
	},
	{	//STR_TRFAC			36
		"Bulgarian / Български",	//LANG_BUL		0
		"Chinese, Simp. / 简体中国",	//LANG_CHS		1
		"Chinese, Trad. / 中国传统",	//LANG_CHT		2
		"English / English",	//LANG_ENG		3
		"French / Français",	//LANG_FRE		4
		"German / Deutsch",	//LANG_GER		5
		"Italian / Italiano",	//LANG_ITL		6
		"Japanese / 日本人",	//LANG_JAP		7
		"Korean / 한국어",	//LANG_KOR		8
		"Polish / Polski",	//LANG_POL		9
		"Portuguese / Português",	//LANG_POR		10
		"Russian / Русский",	//LANG_RUS		11
		"Spanish / Español"	//LANG_SPA		12
	},
	{	//STR_FARM			37
		"Bulgarian / Български",	//LANG_BUL		0
		"Chinese, Simp. / 简体中国",	//LANG_CHS		1
		"Chinese, Trad. / 中国传统",	//LANG_CHT		2
		"English / English",	//LANG_ENG		3
		"French / Français",	//LANG_FRE		4
		"German / Deutsch",	//LANG_GER		5
		"Italian / Italiano",	//LANG_ITL		6
		"Japanese / 日本人",	//LANG_JAP		7
		"Korean / 한국어",	//LANG_KOR		8
		"Polish / Polski",	//LANG_POL		9
		"Portuguese / Português",	//LANG_POR		10
		"Russian / Русский",	//LANG_RUS		11
		"Spanish / Español"	//LANG_SPA		12
	},
	{	//STR_SHMINE			38
		"Bulgarian / Български",	//LANG_BUL		0
		"Chinese, Simp. / 简体中国",	//LANG_CHS		1
		"Chinese, Trad. / 中国传统",	//LANG_CHT		2
		"English / English",	//LANG_ENG		3
		"French / Français",	//LANG_FRE		4
		"German / Deutsch",	//LANG_GER		5
		"Italian / Italiano",	//LANG_ITL		6
		"Japanese / 日本人",	//LANG_JAP		7
		"Korean / 한국어",	//LANG_KOR		8
		"Polish / Polski",	//LANG_POL		9
		"Portuguese / Português",	//LANG_POR		10
		"Russian / Русский",	//LANG_RUS		11
		"Spanish / Español"	//LANG_SPA		12
	},
	{	//STR_IRONSM			39
		"Bulgarian / Български",	//LANG_BUL		0
		"Chinese, Simp. / 简体中国",	//LANG_CHS		1
		"Chinese, Trad. / 中国传统",	//LANG_CHT		2
		"English / English",	//LANG_ENG		3
		"French / Français",	//LANG_FRE		4
		"German / Deutsch",	//LANG_GER		5
		"Italian / Italiano",	//LANG_ITL		6
		"Japanese / 日本人",	//LANG_JAP		7
		"Korean / 한국어",	//LANG_KOR		8
		"Polish / Polski",	//LANG_POL		9
		"Portuguese / Português",	//LANG_POR		10
		"Russian / Русский",	//LANG_RUS		11
		"Spanish / Español"	//LANG_SPA		12
	},
	{	//STR_OILWELL			40
		"Bulgarian / Български",	//LANG_BUL		0
		"Chinese, Simp. / 简体中国",	//LANG_CHS		1
		"Chinese, Trad. / 中国传统",	//LANG_CHT		2
		"English / English",	//LANG_ENG		3
		"French / Français",	//LANG_FRE		4
		"German / Deutsch",	//LANG_GER		5
		"Italian / Italiano",	//LANG_ITL		6
		"Japanese / 日本人",	//LANG_JAP		7
		"Korean / 한국어",	//LANG_KOR		8
		"Polish / Polski",	//LANG_POL		9
		"Portuguese / Português",	//LANG_POR		10
		"Russian / Русский",	//LANG_RUS		11
		"Spanish / Español"	//LANG_SPA		12
	},
	{	//STR_OILREF			41
		"Bulgarian / Български",	//LANG_BUL		0
		"Chinese, Simp. / 简体中国",	//LANG_CHS		1
		"Chinese, Trad. / 中国传统",	//LANG_CHT		2
		"English / English",	//LANG_ENG		3
		"French / Français",	//LANG_FRE		4
		"German / Deutsch",	//LANG_GER		5
		"Italian / Italiano",	//LANG_ITL		6
		"Japanese / 日本人",	//LANG_JAP		7
		"Korean / 한국어",	//LANG_KOR		8
		"Polish / Polski",	//LANG_POL		9
		"Portuguese / Português",	//LANG_POR		10
		"Russian / Русский",	//LANG_RUS		11
		"Spanish / Español"	//LANG_SPA		12
	},
	{	//STR_NUCPOW			42
		"Bulgarian / Български",	//LANG_BUL		0
		"Chinese, Simp. / 简体中国",	//LANG_CHS		1
		"Chinese, Trad. / 中国传统",	//LANG_CHT		2
		"English / English",	//LANG_ENG		3
		"French / Français",	//LANG_FRE		4
		"German / Deutsch",	//LANG_GER		5
		"Italian / Italiano",	//LANG_ITL		6
		"Japanese / 日本人",	//LANG_JAP		7
		"Korean / 한국어",	//LANG_KOR		8
		"Polish / Polski",	//LANG_POL		9
		"Portuguese / Português",	//LANG_POR		10
		"Russian / Русский",	//LANG_RUS		11
		"Spanish / Español"	//LANG_SPA		12
	},
	{	//STR_COALPOW			43
		"Bulgarian / Български",	//LANG_BUL		0
		"Chinese, Simp. / 简体中国",	//LANG_CHS		1
		"Chinese, Trad. / 中国传统",	//LANG_CHT		2
		"English / English",	//LANG_ENG		3
		"French / Français",	//LANG_FRE		4
		"German / Deutsch",	//LANG_GER		5
		"Italian / Italiano",	//LANG_ITL		6
		"Japanese / 日本人",	//LANG_JAP		7
		"Korean / 한국어",	//LANG_KOR		8
		"Polish / Polski",	//LANG_POL		9
		"Portuguese / Português",	//LANG_POR		10
		"Russian / Русский",	//LANG_RUS		11
		"Spanish / Español"	//LANG_SPA		12
	},
	{	//STR_CEMPL			44
		"Bulgarian / Български",	//LANG_BUL		0
		"Chinese, Simp. / 简体中国",	//LANG_CHS		1
		"Chinese, Trad. / 中国传统",	//LANG_CHT		2
		"English / English",	//LANG_ENG		3
		"French / Français",	//LANG_FRE		4
		"German / Deutsch",	//LANG_GER		5
		"Italian / Italiano",	//LANG_ITL		6
		"Japanese / 日本人",	//LANG_JAP		7
		"Korean / 한국어",	//LANG_KOR		8
		"Polish / Polski",	//LANG_POL		9
		"Portuguese / Português",	//LANG_POR		10
		"Russian / Русский",	//LANG_RUS		11
		"Spanish / Español"	//LANG_SPA		12
	},
	{	//STR_CHEMPL			45
		"Bulgarian / Български",	//LANG_BUL		0
		"Chinese, Simp. / 简体中国",	//LANG_CHS		1
		"Chinese, Trad. / 中国传统",	//LANG_CHT		2
		"English / English",	//LANG_ENG		3
		"French / Français",	//LANG_FRE		4
		"German / Deutsch",	//LANG_GER		5
		"Italian / Italiano",	//LANG_ITL		6
		"Japanese / 日本人",	//LANG_JAP		7
		"Korean / 한국어",	//LANG_KOR		8
		"Polish / Polski",	//LANG_POL		9
		"Portuguese / Português",	//LANG_POR		10
		"Russian / Русский",	//LANG_RUS		11
		"Spanish / Español"	//LANG_SPA		12
	},
	{	//STR_ELECPL			46
		"Bulgarian / Български",	//LANG_BUL		0
		"Chinese, Simp. / 简体中国",	//LANG_CHS		1
		"Chinese, Trad. / 中国传统",	//LANG_CHT		2
		"English / English",	//LANG_ENG		3
		"French / Français",	//LANG_FRE		4
		"German / Deutsch",	//LANG_GER		5
		"Italian / Italiano",	//LANG_ITL		6
		"Japanese / 日本人",	//LANG_JAP		7
		"Korean / 한국어",	//LANG_KOR		8
		"Polish / Polski",	//LANG_POL		9
		"Portuguese / Português",	//LANG_POR		10
		"Russian / Русский",	//LANG_RUS		11
		"Spanish / Español"	//LANG_SPA		12
	},
	{	//STR_GASSTN			47
		"Bulgarian / Български",	//LANG_BUL		0
		"Chinese, Simp. / 简体中国",	//LANG_CHS		1
		"Chinese, Trad. / 中国传统",	//LANG_CHT		2
		"English / English",	//LANG_ENG		3
		"French / Français",	//LANG_FRE		4
		"German / Deutsch",	//LANG_GER		5
		"Italian / Italiano",	//LANG_ITL		6
		"Japanese / 日本人",	//LANG_JAP		7
		"Korean / 한국어",	//LANG_KOR		8
		"Polish / Polski",	//LANG_POL		9
		"Portuguese / Português",	//LANG_POR		10
		"Russian / Русский",	//LANG_RUS		11
		"Spanish / Español"	//LANG_SPA		12
	},
	{	//STR_ROAD			48
		"Bulgarian / Български",	//LANG_BUL		0
		"Chinese, Simp. / 简体中国",	//LANG_CHS		1
		"Chinese, Trad. / 中国传统",	//LANG_CHT		2
		"English / English",	//LANG_ENG		3
		"French / Français",	//LANG_FRE		4
		"German / Deutsch",	//LANG_GER		5
		"Italian / Italiano",	//LANG_ITL		6
		"Japanese / 日本人",	//LANG_JAP		7
		"Korean / 한국어",	//LANG_KOR		8
		"Polish / Polski",	//LANG_POL		9
		"Portuguese / Português",	//LANG_POR		10
		"Russian / Русский",	//LANG_RUS		11
		"Spanish / Español"	//LANG_SPA		12
	},
	{	//STR_POWL			49
		"Bulgarian / Български",	//LANG_BUL		0
		"Chinese, Simp. / 简体中国",	//LANG_CHS		1
		"Chinese, Trad. / 中国传统",	//LANG_CHT		2
		"English / English",	//LANG_ENG		3
		"French / Français",	//LANG_FRE		4
		"German / Deutsch",	//LANG_GER		5
		"Italian / Italiano",	//LANG_ITL		6
		"Japanese / 日本人",	//LANG_JAP		7
		"Korean / 한국어",	//LANG_KOR		8
		"Polish / Polski",	//LANG_POL		9
		"Portuguese / Português",	//LANG_POR		10
		"Russian / Русский",	//LANG_RUS		11
		"Spanish / Español"	//LANG_SPA		12
	},
	{	//STR_CRPIPE			50
		"Bulgarian / Български",	//LANG_BUL		0
		"Chinese, Simp. / 简体中国",	//LANG_CHS		1
		"Chinese, Trad. / 中国传统",	//LANG_CHT		2
		"English / English",	//LANG_ENG		3
		"French / Français",	//LANG_FRE		4
		"German / Deutsch",	//LANG_GER		5
		"Italian / Italiano",	//LANG_ITL		6
		"Japanese / 日本人",	//LANG_JAP		7
		"Korean / 한국어",	//LANG_KOR		8
		"Polish / Polski",	//LANG_POL		9
		"Portuguese / Português",	//LANG_POR		10
		"Russian / Русский",	//LANG_RUS		11
		"Spanish / Español"	//LANG_SPA		12
	},
#if 0
	
#endif
	
};
#endif
