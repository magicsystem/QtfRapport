#include "QtfRapport.h"


#define LAYOUTFILE <QtfRapport/QtfRapport.lay>
#include <CtrlCore/lay.h>


class QtfRapportTest : public WithQtfRapportLayout<TopWindow> {
public:
	typedef QtfRapportTest CLASSNAME;
	
	QtfRapportTest();
	void Perform();
	void Print();
	
	bool OnLoop(QRArg& arg);
	bool OnStart(QtfRapport& RAP);
	
	int ii = 0,di=0;
	double total,dtotal;
};


QtfRapportTest::QtfRapportTest()
{
	CtrlLayout(*this, "Window title");
	BTN_LOAD << THISBACK(Perform);
	BTN_PRINT << THISBACK(Print);
	
	Perform();
}



void QtfRapportTest::Perform()
{
	int ii = 0,di=0;
	double total=0,dtotal=0;
	RT.Pick(pick(
		QtfRapport(LoadFile(GetDataFile("template.qtf"))
					,THISBACK(OnStart)
					,THISBACK(OnLoop)
					))
		,GetRichTextStdScreenZoom());
		
}

void QtfRapportTest::Print()
{
	int ii = 0,di=0;
	double total=0,dtotal=0;
	QtfRapport(LoadFile(GetDataFile("template.qtf"))
		,THISBACK(OnStart)
		,THISBACK(OnLoop)
		).Print()
	;
}

bool QtfRapportTest::OnStart(QtfRapport& RAP)
{
	ii = di = 0;
	dtotal = total = 0;
	RAP
		("DATE", GetSysTime())
	;
	return true;
}

bool QtfRapportTest::OnLoop(QRArg& arg)
{
	if(arg.Is("ELM")) {
		ii ++;
		di = 0;
		arg
			("NOM", Format("Nom %d", ii))
			("PRENOM", Format("Prénom %d", ii))
			("TOTALL", Format("%2!,nl", 0.0))
			;
		
		if(ii >= 100 - 1)
			arg.GetAdd("TOTAL") = Format("%2!,nl", total);
		
		if(ii % 2 == 0) arg = "ELMD";
		else arg.SetDefault();
		return ii < 100;
	}
	
	
	else if(arg.Is("DET") ){
		if(di == 0) dtotal = 0;
		di++;
		arg
			("ID", Format("ID %d %d", ii,di))
			("CODE", Format("%2!,nl", di * 10.25 ))
			;
		
		dtotal += di * 10.25;
		total  += di * 10.25;;
		if(di + 1 >= 10) {
			arg.GetAdd("TOTALL") = Format("%2!,nl", dtotal );
		}
		return di<10;
	}
	
	
	else return false;
}




GUI_APP_MAIN
{
	QtfRapportTest().Zoomable().Sizeable().Run();
}

