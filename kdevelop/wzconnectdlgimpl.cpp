/****************************************************************************
 * cclasspropertiesdlgimpl.cpp                                              *
 * Safe Implementation class from cclassproperties.ui                       *
 *--------------------------------------------------------------------------*
 * Author :                                                                 *
 *            Serge Lussier                                                 *
 * email: serge.lussier@videotron.ca                                        *
 * Started on January 15, 2001                                              *
 *--------------------------------------------------------------------------*
 * NOTE:                                                                    *
 * Before re-implement the ui file (with -impl option), make a safe copy of *
 * this file then re-put                                                    *
 * added stuff in this file to the new file.                                *
 ****************************************************************************/

#include "wzconnectdlgimpl.h"

#include "cclasstooldlg.h"
#include "cclonefunctiondlg.h"
#include "./classparser/ClassParser.h"

#include <kdebug.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kmessagebox.h>

#include <qcombobox.h>
#include <qcheckbox.h>
#include <qgroupbox.h>
#include <qlineedit.h>
#include <qlistbox.h>
#include <qlabel.h>
#include <qmultilineedit.h>
#include <qpushbutton.h>
#include <qradiobutton.h>
#include <qmultilineedit.h>
#include <qcheckbox.h>
#include "cclasstooldlg.h"
#include <kmessagebox.h>
#include "cclonefunctiondlg.h"
#include <kiconloader.h>

/* record of the classnames in which their header filename
 * are not built through their name. 
 * To be used in near future...
 */
ClsHeader QtKde[] = {
     { QString("QListViewItem"), QString("qlistview.h") },
     { QString(""), QString("") }
};



#include <qtabwidget.h>


/* 
 *  Constructs a CClassPropertiesDlgImpl which is a child of 'CClassPropertiesDlg', with the
 *  name 'name' and widget flags set to 'f'
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  TRUE to construct a modal dialog.
 */
CClassPropertiesDlgImpl::CClassPropertiesDlgImpl( QWidget* parent,  const char* name, bool, WFlags fl )
    : CClassPropertiesDlg( parent, name, true, fl ), ctpAction ( CTPVIEW )
{
    init();
    classOfSig = 0;
    signalMethod = 0;
    slotMethod = 0;
    theParser = 0;
    implMethod  = 0;
    attrMember = 0;
    slotMethod = 0;
    signalMethod = 0;
}

CClassPropertiesDlgImpl::CClassPropertiesDlgImpl( CTPACTION action, CClassToolDlg* ctdlg, QWidget* parent, const char* name, bool, WFlags fl )
: CClassPropertiesDlg( parent, name, true, fl ), ctpAction(action), CTDlg(ctdlg)
{
    // ToDo: init stuff here...

    classOfSig = 0;
    signalMethod = 0;
    slotMethod = 0;
    theParser = 0;
    implMethod  = 0;
    attrMember = 0;
    slotMethod = 0;
    signalMethod = 0;
    init();

}

CClassPropertiesDlgImpl::CClassPropertiesDlgImpl( CClassView* cv, CTPACTION action, CClassToolDlg* ctdlg, QWidget* parent, const char* name, bool, WFlags fl )
: CClassPropertiesDlg( parent, name, true, fl ), ctpAction(action), CTDlg(ctdlg), class_tree(cv)
{
    // ToDo: init stuff here...

    classOfSig = 0;
    signalMethod = 0;
    slotMethod = 0;
    theParser = 0;
    implMethod  = 0;
    attrMember = 0;
    slotMethod = 0;
    signalMethod = 0;
    init();

}

void CClassPropertiesDlgImpl::resizeEvent( QResizeEvent* e)
{
    QDialog::resizeEvent(e);
    // For a reason I don't know yet, the CClassToolDlg::resizeEvent is not called,
    // even though it is a child widget ...
    //tpgClassView -> resize(tabWidget -> width(), tabWidget -> height());
    //kdDebug() << "new tabWidget Size: W:" << tabWidget -> width() << "H:" << tabWidget -> height() << endl;
}

/*  
 *  Destroys the object and frees any allocated resources
 */
CClassPropertiesDlgImpl::~CClassPropertiesDlgImpl()
{
    // no need to delete child widgets, Qt does it all for us
}

/* 
 * public slot
 */

void CClassPropertiesDlgImpl::applyAddAttribute()
{
  CParsedAttribute *aAttr = new CParsedAttribute();
  QString comment;

  aAttr->setType( leVarType_2 -> text() );
  aAttr->setName( leVarName_2 -> text() );
  aAttr->setDeclaredInScope( currentClass -> path() );
  // Set export
  if( rbVarPublic_2 -> isChecked() )
    aAttr->setExport( PIE_PUBLIC );
  else if( rbVarProtected_2 -> isChecked() )
    aAttr->setExport( PIE_PROTECTED );
  else if( rbVarPrivate_2 -> isChecked() )
    aAttr->setExport( PIE_PRIVATE );

  // Set modifiers
  aAttr->setIsStatic( chVarStatic_2 -> isChecked() );
  aAttr->setIsConst( chVarConst_2 -> isChecked() );

  // Set comment
  comment = "/** " + meVarDoc_2 -> text() + " */";
  aAttr->setComment( comment );

  // update rollback data
  tbdata[CTPATTRIBUTE].editFields[0] = leVarType_2 -> text();
  tbdata[CTPATTRIBUTE].editFields[1] = leVarName_2 -> text();
  tbdata[CTPATTRIBUTE].bModified = false;
  tbdata[CTPATTRIBUTE].bApplied = true;
  emit sigAddAttribute( currentClass -> name.data(), aAttr );
  buttonApply -> setEnabled ( false );
  buttonUndo -> setEnabled ( false );
  //currentClass -> addAttribute ( aAttr ); oops! already done in CKdevelop...
  // causes a crash if applied twice!

}

void CClassPropertiesDlgImpl::applyAddMethod()
{
    int lpPos;
    QString decl;
    CParsedMethod *aMethod = new CParsedMethod();
    QString comment;
    QLineEdit* eType=0, * eName=0;
    QRadioButton * rbPub=0, * rbPro=0, * rbPri=0;
    QCheckBox* chVirtual=0, * chPure=0, * chStatic=0, *chConst=0;
    QMultiLineEdit* meDoc;
    int pg = tabWidget -> currentPageIndex();
    kdDebug() << "applyAddMethod: in page #" << pg << endl;
    switch ( pg )
    {
        case CTPSIGNAL:
            rbPub = rbSigPublic_2;
            rbPro = rbSigProtected_2;
            rbPri   = 0;
            chPure = chVirtual = chStatic = chConst = 0;
            meDoc = meSigDoc_2;
            eType = 0;
            eName = leSigDeclare_2;
            break;
        case CTPSLOT:
            rbPub = rbSlotPublic_2;
            rbPro = rbSlotProtected_3;
            rbPri  = rbSlotProtected_2_2;
            chVirtual = chSlotVirtual_2;
            chConst = chStatic = chPure = 0;
            meDoc = meSlotDoc_2;
            eType = 0;
            eName = leSlotDeclare_2;
            break;
        case CTPMETHOD:
            rbPub = rbMethPublic_2;
            rbPro = rbMethProtected_2;
            rbPri = rbMethPrivate_2;
            chVirtual = chMethVirtual_2;
            chPure = chMethPure_2;
            chStatic = chMethStatic_2;
            chConst = chMethConst_2;
            meDoc = meMethDoc_2;
            eType = leMethType_2;
            eName = leMethDeclare_2;
            break;
        default:
            rbPub = rbPro = rbPri = 0;
            meDoc = 0;
            chVirtual = chPure = chStatic = chConst = 0;
            break;
    }
    aMethod->setType( eType ? eType -> text() : QString("void") );
    aMethod->setDeclaredInScope( currentClass -> path() );
    decl = eName -> text();
    lpPos = decl.find( "(" );
    // If no arguments we add ().
    if( lpPos == -1 )
        aMethod->setName( decl + "()" );
    else // Else just set the whole thing as the name
        aMethod->setName( decl );
    // Set the type.

    if ( pg == CTPSIGNAL )
        aMethod -> setIsSignal( true );
    else if ( pg == CTPSLOT )
        aMethod -> setIsSlot ( true );
    // Set the export.

  if( rbPub && rbPub -> isChecked() )
    aMethod->setExport( PIE_PUBLIC );
  else if( rbPro && rbPro -> isChecked() )
    aMethod->setExport( PIE_PROTECTED );
  else if( rbPri && rbPri -> isChecked() )
    aMethod->setExport( PIE_PRIVATE );

  // Set the modifiers if they are enabled.
  if( chPure && chPure -> isEnabled() )
    aMethod->setIsPure( chPure -> isChecked() );
  if( chStatic && chStatic -> isEnabled() )
    aMethod->setIsStatic( chStatic -> isChecked() );
  if( chConst && chConst -> isEnabled() )
    aMethod->setIsConst( chConst -> isChecked() );
  if( chVirtual && chVirtual -> isEnabled())
    aMethod->setIsVirtual( chVirtual -> isChecked() );

  // Set comment
  if ( meDoc && (meDoc -> text() != "") )
    comment = "/** " + meDoc -> text() + " */";
  else
    comment = "/** No descriptions */";
  aMethod->setComment( comment );
  if (! currentClass )
  {
    kdDebug() << "ERROR currentClass != NULL!!!!!!!" << endl;
    return;
  }
  emit sigAddMethod( currentClass -> path().data(), aMethod);


  //kdDebug() << "Definition of method ends on line: " <<aMethod -> definitionEndsOnLine << endl;
  //delete aMethod;
  tbdata[pg].editFields[0] = eType ? eType -> text() : QString("");
  tbdata[pg].editFields[1] = eName -> text();
  tbdata[pg].bModified = false;
  tbdata[pg].bApplied = true;
  buttonApply -> setEnabled ( false );
  buttonUndo -> setEnabled ( false );

    if ( pg == CTPMETHOD)
    {
        //QList <CParsedMethod> * m = currentClass -> getSortedMethodList();
        //kdDebug() << "class " << currentClass -> name.data() << " : updating methods list..." << endl;
        currentClass -> addMethod( aMethod );
        cbImplMethodList_2 -> insertItem( aMethod -> name) ;
    }
    else
    if ( pg == CTPSLOT )
    {
        //QList <CParsedMethod> * m = currentClass -> getSortedSlotList();
        kdDebug() << "adding new slot to currentClass..." << endl;
        currentClass -> addSlot( aMethod );
        cbSlotMemberList_2 -> insertItem( aMethod -> name );
    }
    if ( pg == CTPSIGNAL )
    {
        kdDebug() << "adding new method to currentClass ..." << endl;
        currentClass -> addSignal ( aMethod );
    }
    // don't touch to signals list...

}


void CClassPropertiesDlgImpl::applySignalSlotMapImplementation()
{
    //QString strSigMethod= "";
    //QString strSlotMethod = "";
    QString strImplMethod = "";
    QString strAttrMember="";
    QString toAdd;
    kdDebug()  << "applying signal-slot connection ..." << endl;
    if ( strSignalMethod.isEmpty() || strSlotMethod.isEmpty() || !implMethod )
    {
        kdDebug() << "Error: caugth NULL POINTER! can't be! uh ?" << endl;
        return;
    }
    //signalMethod -> asString ( strSigMethod );
    kdDebug() << "signalMethod: " << strSignalMethod.data() << endl;
    kdDebug() << "slotMethod: " << strSlotMethod.data() << endl;
    implMethod -> asString ( strImplMethod );
    kdDebug() << "implMethod: " << strImplMethod.data() << endl;

    if ( attrMember ) strAttrMember = attrMember -> name;
    else strAttrMember = "this";

    toAdd = "connect( " + strAttrMember + ", ";
    toAdd = toAdd + "SIGNAL( " + strSignalMethod + " ), ";
    toAdd = toAdd + "SLOT( " + strSlotMethod + " ) );";

    kdDebug() << " toAdd = " << toAdd.data() << endl;

    QString Message;

    if( implMethod -> definitionEndsOnLine < 1 )
    {
        Message = i18n("Unable to locate definition of method") + "\n" +
                       implMethod -> name + "\n" +
                       i18n("Sorry...");
        KMessageBox::error(this, Message, "Implementation method error", true);
        return;
    }
    Message = i18n("You are about to commit connection with this settings:") + "\n" +
                   i18n("Implement in class: ") + currentClass -> name + "\n" +
                   i18n("Attribute member: ") + (attrMember ? attrMember -> name : QString("this")) + "\n" +
                   i18n("Class of attribute member: ") + classOfSig -> name + "\n" +
                   i18n("Signal name: ") + strSignalMethod + "\n" +
                   i18n("Slot method: ") + strSlotMethod + "\n" +
                   i18n("Into method: ") + strImplMethod + " "  + i18n("as") + "\n" +
                   toAdd+ "\n" + i18n("Do you want to continue ?");

    int r = KMessageBox::warningContinueCancel (this, Message, QString( "Signal-Slot " + i18n("connection:")) , i18n("&Continue"));// i18n("&Continue"), i18n("&Cancel"), true);
    if ( r != KMessageBox::Continue ) return;

    emit sigSigSlotMapImplement ( currentClass, toAdd, implMethod );

    buttonApply -> setEnabled ( false );
    buttonUndo -> setEnabled ( false );
    accept();

}



void CClassPropertiesDlgImpl::slotBtnApply()
{
    int pg = tabWidget -> currentPageIndex();
    switch ( pg )
    {
        case CTPATTRIBUTE:
            kdDebug() << "Applying new Attribute..." << endl;
            applyAddAttribute();
            accept();
            return;
        case CTPMETHOD:
        case CTPSIGNAL:
        case CTPSLOT:
            kdDebug() << "Applying new method..." << endl;
            applyAddMethod();
            accept();
            return;
        case CTPIMPL:
            applySignalSlotMapImplementation();
            accept();
            break;
        default:
            buttonApply -> setEnabled( false );
            buttonUndo -> setEnabled ( false );
            break;
    }
}

/*
 * public slot
 */
void CClassPropertiesDlgImpl::slotBtnUndo()
{
    qWarning( "CClassPropertiesDlgImpl::slotBtnUndo() not yet implemented!" );
}

/*
 * protected slot
 */
void CClassPropertiesDlgImpl::slotSlotModifierChanged(int )
{
    qWarning( "CClassPropertiesDlgImpl::slotSlotModifierChanged(int ) not yet implemented!" );
}
/*
 * protected slot
 */
void CClassPropertiesDlgImpl::slotAddSlotState(int)
{
    if (chSlotAdd -> isChecked())
    {
        gbAddNewSlot -> setEnabled(true);
        gbSlotMembers -> setEnabled(false);
        QString strDeclare = "new_slot(";
        QString strArgs = "";
        if ( signalMethod )
        {
            QList <CParsedArgument>  argList = signalMethod -> arguments;
            kdDebug() << "build args from signal " << strSignalMethod.data() << endl;
            CParsedArgument * arg;
            if ( argList. count() > 0 )
            {
                for ( arg = argList.first(); arg; arg = argList.next())
                {
                    if ( strArgs != "" ) strArgs = strArgs + ", ";
                    strArgs = strArgs + arg -> type;
                }
                kdDebug() << "args are " << strArgs.data() << endl;
            }
        }
        strDeclare = strDeclare + strArgs + ")";
        leSlotDeclare_2 -> setText ( strDeclare );
    }
    else
    {
        gbAddNewSlot -> setEnabled(false);
        gbSlotMembers -> setEnabled(true);
        buttonUndo -> setEnabled (false);
        buttonApply -> setEnabled(false);
    }
}
/*
 * protected slot
 */
void CClassPropertiesDlgImpl::slotImplMethodSelected(const QString& strMethod)
{
    bool btrue;
    QList < CParsedMethod> * l =  currentClass -> getSortedMethodList();
    QString am;
    kdDebug() << "in slotImplMethodSelected:" << endl;
    kdDebug() << "getting instance of " << strMethod.data() << endl;
    for ( implMethod = l -> first() ; implMethod; implMethod = l -> next())
    {
        implMethod -> asString ( am );
        kdDebug() << "mathing " << am.data() << endl;
        if ( am == strMethod ) break;
    }
    if ( implMethod == NULL )
    {
        kdDebug() << "no methods... trying in slots list..." << endl;
        l = currentClass -> getSortedSlotList();
        for ( implMethod = l -> first() ; implMethod; implMethod = l -> next())
        {
            implMethod -> asString ( am );
            kdDebug() << "matching " << am.data() << endl;
            if ( (am == strMethod) && (strMethod != implMethod -> name) ) break;
        }
    }

    //implMethod = currentClass -> getMethodByNameAndArg( strMethod.data() );
    if ( signalMethod && slotMethod && implMethod ) btrue = true;
    else btrue = false;
    buttonApply -> setEnabled(btrue);
    buttonUndo -> setEnabled(btrue);

    if ( signalMethod == NULL ) kdDebug() << " signal method is NULL! " << endl;
    if ( slotMethod == NULL ) kdDebug() << " slot method is NULL! " << endl;
    if ( implMethod == NULL ) kdDebug() << " impl. methodis NULL! " << endl;
}
/*
 * protected slot
 */
void CClassPropertiesDlgImpl::slotMethAccessChanged(int)
{
    qWarning( "CClassPropertiesDlgImpl::slotMethAccessChanged(int) not yet implemented!" );
}
/*
 * protected slot
 */
void CClassPropertiesDlgImpl::slotMethModifierChanged(int)
{
    qWarning( "CClassPropertiesDlgImpl::slotMethModifierChanged(int) not yet implemented!" );
}
/*
 * protected slot
 */
void CClassPropertiesDlgImpl::slotMethNameChanged( const QString & )
{
    if( tbdata[CTPMETHOD].bModified == true) return;
    tbdata[CTPMETHOD].bModified = true;
    tbdata[CTPMETHOD].bApplied = false;
    //tbdata.bUndo = false;
    buttonApply -> setEnabled(true);
    buttonUndo -> setEnabled(true);
}
/*
 * protected slot
 */
void CClassPropertiesDlgImpl::slotMethTypeChanged( const QString & )
{
    if( tbdata[CTPMETHOD].bModified == true) return;
    tbdata[CTPMETHOD].bModified = true;
    tbdata[CTPMETHOD].bApplied = false;
    //tbdata.bUndo = false;
    buttonApply -> setEnabled(true);
    buttonUndo -> setEnabled(true);
}
/*
 * protected slot
 */
void CClassPropertiesDlgImpl::slotSigAccessChanged(int)
{
    qWarning( "CClassPropertiesDlgImpl::slotSigAccessChanged(int) not yet implemented!" );
}
/*
 * protected slot
 */
void CClassPropertiesDlgImpl::slotSigAddSignalState(int)
{
    if (chSigAdd_2 -> isChecked())
    {
        gbAddNewSig_2 -> setEnabled(true);
        GroupBox9_2 -> setEnabled(false);
        if ( leSigDeclare_2 -> text() == "" )
            leSigDeclare_2 -> setText ( "new_signal()" );
    }
    else
    {
        gbAddNewSig_2 -> setEnabled(false);
        GroupBox9_2 -> setEnabled(true);
        buttonUndo -> setEnabled (false);
        buttonApply -> setEnabled(false);
    }


}
/*
 * protected slot
 */
void CClassPropertiesDlgImpl::slotSigMemberSelected(const QString& aMember)
{
    QString aName;
    QString a;
    getClassNameFromString( aMember, aName );
    getMemberFromString( aMember, a);

    CParsedClass * aClass;
    //cbSigSignalList_2 -> clear();
    if ( theParser != NULL )
    {// Clean up old parser....
        delete theParser;
        theParser = NULL;
    }
    //kdDebug() <<  "class to get signal list '" << aName.data() <<"'" << "curerntClass:'"<< currentClass -> name << "'" << (int) (aName == currentClass -> name) <<  endl;
    leSigClassName -> setText ( aName );
    if (aName == currentClass -> name)
    {
        classOfSig = currentClass;
        //setSignalsMemberList( currentClass );
        fillSignalCombo( currentClass );
        selectedSigAttrClass = currentClass;
        return;
    }

    aClass = store -> getClassByName ( aName );
    if ( aClass == NULL ){
        aClass = unParsedClass( aName );
        selectedSigAttrClass = aClass;
        if( aClass == NULL)
        {
            kdDebug() << " CClassPropertiesDlgImpl::slotSigMemberSelected: failed to parse..." << endl;
            return;
        }
    }
    classOfSig = aClass;
    //setSignalsMemberList ( aClass );
    fillSignalCombo ( aClass );

}
/*
 * protected slot
 */
void CClassPropertiesDlgImpl::slotSigNameChanged( const QString& )
{
    if( tbdata[CTPSIGNAL].bModified == true) return;
    tbdata[CTPSIGNAL].bModified = true;
    tbdata[CTPSIGNAL].bApplied = false;
    //tbdata.bUndo = false;
    buttonApply -> setEnabled(true);
    buttonUndo -> setEnabled(true);
}
/*
 * protected slot
 */
void CClassPropertiesDlgImpl::slotSigSignalSelected(const QString& aName)
{
    CParsedMethod* meth;
    QList <CParsedMethod>* methList;
    if (classOfSig == 0){ kdDebug() << "no member class selected,..." << endl;  return; }
    meth  = classOfSig -> getSignalByNameAndArg( aName.data() );
    strSignalMethod = aName;
    signalMethod = meth;
    //if ( meth == 0 ) return;
    methList = currentClass -> getSortedSlotList();
    setSlotTabSlotList ( methList, true);
    kdDebug() << "CClassPropertiesDlgImpl::slotSigSignalSelected() - signal '" << strSignalMethod.data() << "' selected.." << endl;
    delete methList;

}
/*
 * protected slot
 */
void CClassPropertiesDlgImpl::slotSlotAccessChanged(int)
{
    if( tbdata[CTPSLOT].bModified || tbdata[CTPSLOT].bApplied ) return;

    qWarning( "CClassPropertiesDlgImpl::slotSlotAccessChanged(int) not yet implemented!" );
}
/*
 * protected slot
 */
void CClassPropertiesDlgImpl::slotSlotMemberSelected(const QString&)
{
    slotMethod = currentClass -> getSlotByNameAndArg ( cbSlotMemberList_2 -> currentText().data() );
    kdDebug() << "slot member " << (slotMethod ? slotMethod -> name.data() : "oops - NULL") << "selected..." << endl;
}
/*
 * protected slot
 */
void CClassPropertiesDlgImpl::slotSlotNameChanged( const QString& )
{
    if( tbdata[CTPSLOT].bModified == true) return;
    tbdata[CTPSLOT].bModified = true;
    tbdata[CTPSLOT].bApplied = false;
    //tbdata.bUndo = false;
    buttonApply -> setEnabled(true);
    buttonUndo -> setEnabled(true);
}
/*
 * protected slot
 */
void CClassPropertiesDlgImpl::slotVarNameChanged( const QString& )
{
    if( tbdata[CTPATTRIBUTE].bModified == true) return;
    tbdata[CTPATTRIBUTE].bModified = true;
    tbdata[CTPATTRIBUTE].bApplied = false;
    //tbdata.bUndo = false;
    buttonApply -> setEnabled(true);
    buttonUndo -> setEnabled(true);
}
/*
 * protected slot
 */
void CClassPropertiesDlgImpl::slotVarTypeChanged( const QString& )
{
    if( tbdata[CTPATTRIBUTE].bModified == true) return;
    tbdata[CTPATTRIBUTE].bModified = true;
    tbdata[CTPATTRIBUTE].bApplied = false;
    //tbdata.bUndo = false;
    buttonApply -> setEnabled(true);
    buttonUndo -> setEnabled(true);
}


/**  */
void CClassPropertiesDlgImpl::setCurrentClassName ( const QString& aClassName )
{
    lbWorkingClass -> setText (QString (i18n("Working on class: ")) + aClassName);
}
/**  */
void CClassPropertiesDlgImpl::slotClassViewChanged( CParsedClass* aClass)
{
    setClass ( aClass );
}

void CClassPropertiesDlgImpl::slotTabChanged( QWidget* )
{// don;t need QWidget* arg

    int i;
    bool btrue;
    i = tabWidget -> currentPageIndex();
    switch ( i )
    {
        case 0: // ClassView
            buttonUndo -> setEnabled(false);
            buttonApply -> setEnabled(false);
            break;
        case 1: // Attributes
        case 2: // Methods
        case 3: // Signals
        case 4: // Slots
            btrue = tbdata[i].bModified;
            buttonApply -> setEnabled(btrue);
            buttonUndo -> setEnabled(btrue);
            break;
        case 5:
            if ( signalMethod && slotMethod && implMethod ) btrue = true;
            else btrue = false;
            buttonApply -> setEnabled(btrue);
            buttonUndo -> setEnabled(btrue);
    }
}
/** Check if data onscreen is the same as its rollback data.
@return true if diff.
 */
bool CClassPropertiesDlgImpl::isModified( int  )
{
    return false;
}
/** Fill'in data into the widgets depending on which TAB ' vtab ' is invoqued */
void CClassPropertiesDlgImpl::updateData( int  )
{

}
/**  */
void CClassPropertiesDlgImpl::init()
{
    int x;
    sigClassList.setAutoDelete( false );
    sigClassList.clear();
    strSlotMethod = "";
    strSignalMethod = "";


    // Do I need this ?
    WidgetTable [0] = tpgClassView;
    WidgetTable [1] = tpgVars;
    WidgetTable [2] = tpgMeth;
    WidgetTable [3] = tpgSignals;
    WidgetTable [4] = tpgSlots;
    WidgetTable [5] = tpgImpl;

    tabWidget -> changeTab (tpgClassView, SmallIcon("CVclass"), "Class Viewer");
    tabWidget -> changeTab (tpgVars, SmallIcon("CVglobal_var"), "Attributes");
    tabWidget -> changeTab (tpgMeth, SmallIcon("CVpublic_meth"), "Methods");
    tabWidget -> changeTab (tpgSignals, SmallIcon("CVpublic_signal"), "Signals");
    tabWidget -> changeTab (tpgSlots, SmallIcon("CVpublic_slot"), "Slots");
    tabWidget -> changeTab (tpgImpl, SmallIcon("CVstruct"), "Connection Implementation");


    // -------------------

    for ( x = 0; x < 6; x++ )
    {
        tbdata[x].editFields[0] = "";
        tbdata[x].editFields[1] = "";
        tbdata[x].combos[0] = "";
        tbdata[x].combos[1] = "";
        tbdata[x].bModified = false;
        tbdata[x].bApplied = false;
    }
    switch ( ctpAction )
    {
        case CTPVIEW:
            break;
        case CTPADDATTR:
            tabWidget -> setCurrentPage( CTPATTRIBUTE );
            break;
        case CTPADDMETH:
            tabWidget -> setCurrentPage( CTPMETHOD );
            break;
        case CTPADDSIGNAL:
            tabWidget -> setCurrentPage( CTPSIGNAL );
            chSigAdd_2 -> setChecked(true);
            slotSigAddSignalState(0);
            break;
        case CTPADDSLOT:
            tabWidget -> setCurrentPage( CTPSLOT );
            chSlotAdd -> setChecked(true);
            slotAddSlotState(0);
            break;
        default:
            break;
    }
    setMinimumSize( 540, 500 );
}
/**  */
void CClassPropertiesDlgImpl::setClass ( CParsedClass* aClass )
{
    currentClass = aClass;
    setCurrentClassName( aClass -> name );
    // Propagate change to the rest of the dialog data
    // Get attributes list
    QList<CParsedAttribute> *AttrList = aClass->getSortedAttributeList();
    // update Signals Tab data:
    setSigTabAttrList ( AttrList );
    // Get slot list:
    QList<CParsedMethod> *MethList = aClass ->getSortedSlotList();
    // update Slots Tab data:
    setSlotTabSlotList ( MethList );
    // Show all slot methods for now ....
    setImplTabMethList ( MethList );
    // Get Methods:
    MethList = aClass -> getSortedMethodList();
    // update Connect Implementation Tab data:
    setImplTabMethList ( MethList,false );
    delete MethList;
    delete AttrList;
}
/**  */
void CClassPropertiesDlgImpl::setSigTabAttrList ( QList <CParsedAttribute>* AttrList )
{
    CParsedAttribute * Attr;
    int apos,pos;
    QString strType, strItem;
    if (AttrList == NULL ) return;
    cbSigMemberList_2 -> clear();
    strItem = currentClass -> name + "* this";
    cbSigMemberList_2 -> insertItem ( strItem );
    if ( AttrList -> count() == 0 )
    {
        kdDebug() << "no attributes in class "<< currentClass -> name.data () << endl;
        return;
    }
    for ( Attr = AttrList -> first() ; Attr; Attr = AttrList -> next())
    {
        strType = Attr -> type;
        //kdDebug() << "type of " << Attr -> name .data() << " is " << strType.data() << endl;
        apos = strType.find('*'); // remove pointer modifier if any
        if(apos != -1) strType.remove(apos,1);
        // remove all spaces from type name!
        // remove modifiers 'const', 'static' from name: we don;t need it...
        if( (pos = strType.find("const")) != -1) strType.remove(pos, 5);
        if( (pos = strType.find("static")) != -1) strType.remove(pos, 6);
        if( (pos = strType.find("[]")) != -1) strType.remove(pos, 2);
        while ( (pos = strType.find(" ")) != -1) strType.remove(pos,1);
        pos = -1;
        if ( !store -> hasClass( strType.data() ) )
        {
            //kdDebug() << strType.data () << " not parsed ..." << endl;
            // here I am screwed for the moment...
            if ( (pos = strType.find('Q')) == 0)
            {
                // Hope this is a QT class....
                //if(apos != -1 ) strType = strType + "*";
                // pix = SmallIcon("CV_QT");
            }
            else
            if ( (pos = strType.find('K')) == 0)
            {
                // Hope this is a KDE class....
                //if(apos != -1 ) strType = strType + "*";
                // pix = SmallIcon("CV_KDE");
            }
            else continue;
        }
        if(apos != -1) strType = strType + "*";
        strItem = strType + " " + Attr -> name;
        cbSigMemberList_2 -> insertItem ( strItem );
    }
     leSigClassName -> setText( currentClass -> name );
}
/** Fill in current class's slots member in ComboBox :*/
void CClassPropertiesDlgImpl::setSlotTabSlotList ( QList<CParsedMethod> * list, bool bmatchsig)
{
    CParsedMethod* meth;
    cbSlotMemberList_2 -> clear();
    QString str;
    QList <CParsedArgument> sigArgs;
    QList <CParsedArgument> slotArgs;
    CParsedArgument *sigArgIt;
    CParsedArgument *slotArgIt;
    bool match = false;
    if(!list) return;
    if ( list -> isEmpty())
    {
        kdDebug() << "no slot members ...." << endl;
        //signalMethod = NULL;
        //slotMethod = 0;
        strSlotMethod = "";
        strSignalMethod = "";
        return;
    }
    if ( strSignalMethod.isEmpty() && bmatchsig )
    {
        kdDebug() << "can't match signal arguments : signal is null" << endl;
        lbSlotClassOfSig -> setText ( i18n("no matching slots") );
        strSlotMethod = "";
        kdDebug() << "slotMethod set to NULL " << endl;
        return;
    }
    kdDebug() << "have strSignalMethod = '" << strSignalMethod.data() << "'" << endl;
    if( bmatchsig && !strSignalMethod.isEmpty() )
    {   // Find the class where the signal method belongs:
        CParsedClass* aClass = NULL;
        signalMethod = NULL;
        if (!sigClassList.isEmpty())
            for ( aClass = sigClassList.first(); aClass; aClass = sigClassList.next())
            {
                kdDebug() << "checking in class " << aClass -> name.data() << endl;
                if( (signalMethod = aClass -> getSignalByNameAndArg( strSignalMethod )) != NULL) break;
            }
    }
    if( !signalMethod )
    {
        kdDebug() << "signalMethod not set or not found..." << endl;
        return;
    }
    sigArgs = signalMethod -> arguments;
    for (meth = list -> first() ; meth; meth = list -> next())
    {
        if( bmatchsig )
        {
            match = false;
            slotArgs = meth -> arguments;
            //kdDebug() << meth -> name.data()  << "numargs: " << slotArgs . count() << endl;
            if( sigArgs . count() ==0 && slotArgs . count() ==0 ) goto ARGMATCH;
            if ( sigArgs . count() != slotArgs . count()) continue;
            for ( sigArgIt = sigArgs . first(),
                   slotArgIt = slotArgs . first();
                   sigArgIt;
                   sigArgIt = sigArgs . next(),
                   slotArgIt = slotArgs . next())
            {
                //kdDebug() << "mathing " << sigArgIt -> type << " with " << slotArgIt -> type << endl;
                if ( sigArgIt -> type == slotArgIt -> type)
                {
                    match = true;
                    break;
                }
            }
            if (!match) continue;
        }
        ARGMATCH:
        meth -> asString( strSlotMethod );
        cbSlotMemberList_2 -> insertItem ( strSlotMethod );
    }
    if( !cbSlotMemberList_2 -> count() )
    {
        kdDebug() << "No matching slot members found..." << endl;
        lbSlotClassOfSig -> setText (i18n("no matching slots"));
        QString Message = i18n("class") + " " + currentClass -> name + " " + i18n("has no slots matching signal declaration") + "\n"+
                           "' " + strSignalMethod + " '";
        KMessageBox::sorry(this, Message, i18n("Slots"));
        slotMethod = 0;
        strSlotMethod = "";
        kdDebug() << "slotMethod set to NULL " << endl;
        return;
    }
    else lbSlotClassOfSig -> setText ( classOfSig ? classOfSig -> name + "'s signals": QString(i18n("no signal selected")) );
    kdDebug() << "getting slot method instacne..." << endl;
    slotMethod = currentClass -> getSlotByNameAndArg( cbSlotMemberList_2 -> currentText().data() );
    kdDebug() << "slotMethod set to '" << slotMethod -> name.data() << endl;
}
/**  */
void CClassPropertiesDlgImpl::setImplTabMethList ( QList<CParsedMethod>* mlist, bool bclear)
{
    CParsedMethod* meth;
    if(bclear) cbImplMethodList_2 -> clear();
    QString strMeth;
    for ( meth = mlist -> first(); meth; meth = mlist -> next())
    {
        meth -> asString( strMeth );
        //kdDebug() << "method " << strMeth.data() << " ends on line#" << meth -> definitionEndsOnLine << endl;
        cbImplMethodList_2 -> insertItem ( strMeth );
    }
    implMethod = currentClass -> getMethodByNameAndArg ( cbImplMethodList_2 -> currentText().data() );
    kdDebug() << "implMethod set to " << (implMethod ? implMethod -> name.data() : "NULL") << endl;
}
/** EditEnter:
      This method tries to get the header filename from the line edit text
      then parse the file. If the file is parsed successfully, then
      it gets its signal member methods if any, then build
      the signals comboxbox with that list.
      The signal method selected from that list will be taken as
      if this is a member of the selected Working classe's attribute
      member.

      This erratic behaviour is set until KDE/QT classes will be actually part of
      store data.
*/
void CClassPropertiesDlgImpl::slotSigClassNameEditEnter()
{
    QString aClassName = leSigClassName -> text();
    CParsedClass* fixClass = unParsedClass( aClassName );
/*
    CParsedClass* guestClass;
    CParsedClass* parentClass;
*/
    if (! fixClass )
    { // abort...
        kdDebug() << " no class or can't get header file of class " << aClassName.data() << endl;
        delete theParser;
        theParser = NULL;
        return;
    }
    /*
    guestClass = theParser -> store . getClassByName (  classOfSig -> name );
    if ( ! guestClass )
    {
        kdDebug() << " No class named '" << classOfSig -> name.data() << "' in parsed file!" << endl;
        kdDebug() << "trying to get parent class of  " << classOfSif -> name.data() << endl;
        if ( fixClass -> hasParent ( aClassName ) )
        {
            parentClass
        delete theParser;
        theParser = NULL;
        return;
    }
    */
    classOfSig = fixClass;
    leSigClassName -> setText ( classOfSig -> name );
    //setSignalsMemberList( classOfSig );
    fillSignalCombo ( classOfSig );
}
/**  */
void CClassPropertiesDlgImpl::getClassNameFromString( const QString & aName, QString& newName)
{
    int pos;
    if( aName.find(" ") != -1) newName = aName.left( aName.find(" ") + 1);
    else
    {
        newName = aName;
        return;
    }
    if ( (pos = newName.find("*")) != -1) newName.remove(pos,1);
    while ( (pos = newName.find(" ")) != -1) newName.remove(pos, 1);
    if ((pos = newName.find("[]")) != -1) newName.remove(pos, 2);
}
/**  */
void CClassPropertiesDlgImpl::setSignalsMemberList( CParsedClass* aClass, bool bClear)
{
    QList<CParsedMethod> *siglist = aClass -> getSortedSignalList();
    QString item;
    if ( bClear ) cbSigSignalList_2 -> clear();
    if ( (siglist == (QList<CParsedMethod> *)0) || ( siglist -> isEmpty()) )
    {
        kdDebug() << "CClassPropertiesDlgImpl::setSignalsMemberList() : class " << aClass -> name.data() << " has no signal members." << endl;
        // ToDo: KMessageBox::warning(...)...
        signalMethod = 0;
        kdDebug() << "signalMethod set to NULL" << endl;
        return;
    }
    CParsedMethod* sig;
    for ( sig = siglist -> first() ; sig; sig = siglist -> next())
    {
        sig -> asString( item );
        cbSigSignalList_2 -> insertItem( item );
    }
    signalMethod = aClass -> getSignalByNameAndArg ( cbSigSignalList_2 -> currentText().data() );
    strSignalMethod = cbSigSignalList_2 -> currentText();
    if ( signalMethod ) kdDebug() << " signalMethod set to '" << signalMethod -> name.data() << "'" << endl;
}
/** This function tries guess if aName is a QT or a KDE class then set filename
      of the include file according to the classname.
      Code below needs some optimization...
      This function only checks for QT or KDE classes...
 */
CParsedClass* CClassPropertiesDlgImpl::unParsedClass( const QString& aName)
{
    CClassParser *parser =  new CClassParser;
    config = KGlobal::config();
    config -> setGroup("QT2");
    CParsedClass* theClass=0l;
    QString qt2dir = config -> readEntry("qt2dir");
    QString kde2dir  = config -> readEntry("kde2dir");
    QString filePath;
    if ( aName.find("Q") == 0 )
        filePath = qt2dir + "/include/" + aName.lower() + ".h";
    else
    if ( aName.find("K") == 0 )
        filePath = kde2dir + "/include/" + aName.lower() + ".h";

    kdDebug() << "about to parse file " << filePath.data() << endl;
    parser -> parse( filePath.data() );
    if (! parser -> store.hasClass( aName ))
    { // Last chance to get the right file: match ClsHeader
        kdDebug() << "Parse failed for " << aName.data() << endl;
        int I, found=0;
        for (I = 0; !QtKde[I].ClsName.isEmpty(); I++)
        {
            if (QtKde[I].ClsName == aName)
            {
                if ( aName.find("Q") == 0 )
                    filePath = qt2dir + "/include/" + QtKde[I].Filename;
                else
                if ( aName.find("K") == 0 )
                    filePath = kde2dir + "/include/" + QtKde[I].Filename;
                else break;
                kdDebug() << "about to parse file " << filePath.data() << endl;
                parser -> parse( filePath.data() );
                found = 1;
                break;
            }
        }
        if (!found)
        {
            delete parser;
            return NULL;
        }
    }
    // Woohoo! we've got it...
    theClass = parser -> store.getClassByName( aName );
    theParser = parser;
    return theClass;
}

void CClassPropertiesDlgImpl::slotMethVirtualState(int)
{
    if ( chMethVirtual_2 -> isChecked() )
    {
        chMethPure_2 -> setEnabled ( true );
        return;
    }
    chMethPure_2 -> setChecked( false );
    chMethPure_2 -> setEnabled( false );
}

/**  */
void CClassPropertiesDlgImpl::getMemberFromString ( const QString& str, QString& newName)
{
    int spos;
    CParsedAttribute* aAttr;
    QList < CParsedAttribute>  *l;
    l = currentClass -> getSortedAttributeList();
    spos = str.length() - str.find(" ");
    //if( spos == -1) spos = 0;
    newName = str.right(spos-1);
    kdDebug() << newName.data() << endl;
    for ( aAttr = l -> first(); aAttr; aAttr = l  -> next())
    {
        if (aAttr -> name == newName)
        {
            attrMember = aAttr;
            kdDebug() << "Atttribute member match: " << attrMember -> name.data() << endl;
            return;
        }
    }
    attrMember = NULL;
}
/**  */
void CClassPropertiesDlgImpl::viewChildren()
{
    CTDlg -> viewChildren();
}
/**  */
void CClassPropertiesDlgImpl::viewParents()
{
    CTDlg -> viewParents();
}
/**  */
void CClassPropertiesDlgImpl::setClassToolDlg( CClassToolDlg* ct)
{
    CTDlg = ct;
}
void CClassPropertiesDlgImpl::slotClone()
{
CCloneFunctionDlg volDlg(class_tree, this, "volnameDlg");

  if (volDlg.exec()) {
    CParsedMethod* res = volDlg.getMethod();
    if (! res)
    	return;
   	
   // copy type and declaration
   QString str;
   leMethType_2 -> setText(res->type);;
   leMethDeclare_2 -> setText(res->asString(str));
      // the comment needs some adjustment
   str = res->comment;
   // remove /** and */
   str.replace( QRegExp("^/\\** *"), "" );
   str.replace( QRegExp(" *\\**/$"), "" );
   // clean up line breaks
   str.replace( QRegExp("\n *\\** *"), "\n" );
	meMethDoc_2 -> setText(str);

    	// all the buttons
   	rbMethPrivate_2  -> setChecked( res->isPrivate() );
    	rbMethProtected_2 -> setChecked( res->isProtected() );
    	rbMethPublic_2 -> setChecked( res->isPublic() );
     	
   	//methodRb.setChecked( true ); // ??
    	//slotRb.setChecked( res->isSlot );
    	//signalRb.setChecked( res->isSignal );

    	chMethConst_2 -> setChecked( res->isConst);
    	chMethStatic_2 -> setChecked( res->isStatic );
    	chMethPure_2 -> setChecked( res->isPure );
    	chMethVirtual_2 -> setChecked( res->isVirtual );
  }
}	



/** This function will try to fill up the combo with 
avaible Signals/Slots methods from the class itself and, if possible, 
from its parent classes. */
bool CClassPropertiesDlgImpl::fillSignalCombo(CParsedClass* aClass, bool bClear)
{
    if (! aClass ) return false;
    CParsedMethod* meth;
    QList <CParsedMethod> *mList;
    CParsedParent* parent;
    CParsedClass* Class;
    QList <CParsedParent> prList;
    QString asString;
    if( bClear )
    {
        cbSigSignalList_2 -> clear();
        sigClassList.clear();
    }

    prList  = aClass -> parents;
    if( prList.count() > 0)
    {
        Class = NULL;
        for ( parent = prList.first(); parent; parent = prList.next())
        {
            Class = NULL;
            if ( (parent -> name == "QWidget") || (parent -> name == "QObject") ) break;
            kdDebug() << "found class " << parent -> name.data() << "as parent of " << aClass -> name.data() << endl;
            Class = store -> hasClass( parent -> name ) ?
                    store -> getClassByName( parent -> name )
                    : unParsedClass ( parent -> name );
            if(!Class) continue;
            fillSignalCombo( Class, false);
        }
    }
    mList = aClass -> getSortedSignalList();
    if (!mList || ( mList -> count() == 0 ) )
    {
        kdDebug()  << "no signals in class " << aClass -> name.data() << endl;
        //QString Message = i18n("There are no signal members in class") + " " + aClass -> name;
        //KMessageBox::sorry(this, Message, i18n("Signals"));
        return false;
    }
    sigClassList.append (aClass);
    for ( meth = mList -> first(); meth; meth = mList -> next())
    {
        meth -> asString ( asString );
        cbSigSignalList_2 -> insertItem( asString );
    }
    return true;
}



//#include "wzconnectdlgimpl.moc"
