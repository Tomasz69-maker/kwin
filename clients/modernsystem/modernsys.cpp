#include "modernsys.h"
#include <qapplication.h>
#include <qcursor.h>
#include <qabstractlayout.h>
#include <qlayout.h>
#include <qtoolbutton.h>
#include <qlabel.h>
#include <qdrawutil.h>
#include <kpixmapeffect.h>
#include <kdrawutil.h>
#include <kapp.h>
#include <qbitmap.h>
#include "../../workspace.h"
#include "../../options.h"

#include "buttondata.h"
#include "btnhighcolor.h"

using namespace KWinInternal;

extern "C"
{
    Client *allocate(Workspace *ws, WId w, int)
    {
        return(new ModernSys(ws, w));
    }
}


static unsigned char iconify_bits[] = {
  0x00, 0x00, 0xff, 0xff, 0x7e, 0x3c, 0x18, 0x00};

/* not used currently
static unsigned char close_bits[] = {
    0xc3, 0x66, 0x3c, 0x18, 0x3c, 0x66, 0xc3, 0x00};
*/
static unsigned char maximize_bits[] = {
  0x00, 0x18, 0x3c, 0x7e, 0xff, 0xff, 0x00, 0x00};

static unsigned char minmax_bits[] = {
    0x0c, 0x18, 0x33, 0x67, 0xcf, 0x9f, 0x3f, 0x3f};

static unsigned char unsticky_bits[] = {
   0x3c, 0x42, 0x99, 0xbd, 0xbd, 0x99, 0x42, 0x3c};

static unsigned char sticky_bits[] = {
   0x3c, 0x42, 0x81, 0x81, 0x81, 0x81, 0x42, 0x3c};

static unsigned char question_bits[] = {
    0x3c, 0x66, 0x60, 0x30, 0x18, 0x00, 0x18, 0x18};

static unsigned char btnhighcolor_mask_bits[] = {
 0xe0,0x41,0xf8,0x07,0xfc,0x0f,0xfe,0xdf,0xfe,0x1f,0xff,0x3f,0xff,0xff,0xff,
 0x3f,0xff,0x3f,0xff,0xff,0xff,0xff,0xfe,0x9f,0xfe,0x1f,0xfc,0x0f,0xf0,0x03,
 0x00,0x40,0x80,0x00,0x00,0x00,0x39,0x00,0x00,0x00,0x20,0x99,0x0f,0x08,0xc4,
 0x00,0x00,0x00,0x67,0x00,0x00,0x00,0x58,0x5f,0x43,0x68,0x61,0x6e,0x67,0x65 };

static KPixmap *aUpperGradient=0;
static KPixmap *iUpperGradient=0;
static QPixmap *buttonPix=0;
static QPixmap *buttonPixDown=0;
static QPixmap *iButtonPix=0;
static QPixmap *iButtonPixDown=0;

static QColor buttonFg;
static bool pixmaps_created = false;

static void make_button_fx(const QColorGroup &g, QPixmap *pix, bool light=false)
{
    static QBitmap lcDark1(14, 15, lowcolor_6a696a_bits, true);
    static QBitmap lcDark2(14, 15, lowcolor_949194_bits, true);
    static QBitmap lcDark3(14, 15, lowcolor_b4b6b4_bits, true);
    static QBitmap lcLight1(14, 15, lowcolor_e6e6e6_bits, true);
    static QImage btnSource(btnhighcolor_xpm);

    pix->fill(g.background());
    QPainter p(pix);

    if(QPixmap::defaultDepth() > 8){
        int i, destH, destS, destV, srcH, srcS, srcV;
        QColor btnColor = g.background();

        if(btnSource.depth() < 32)
            btnSource = btnSource.convertDepth(32);
        if(light)
            btnColor = btnColor.light(120);
        btnColor.hsv(&destH, &destS, &destV);
        QImage btnDest(14, 15, 32);

        unsigned int *srcData = (unsigned int *)btnSource.bits();
        unsigned int *destData = (unsigned int *)btnDest.bits();
        QColor srcColor;
        for(i=0; i < btnSource.width()*btnSource.height(); ++i){
            srcColor.setRgb(srcData[i]);
            srcColor.hsv(&srcH, &srcS, &srcV);
            srcColor.setHsv(destH, destS, srcV);
            destData[i] = srcColor.rgb();
        }
        pix->convertFromImage(btnDest);

    }
    else{
        if(!lcDark1.mask()){
            lcDark1.setMask(lcDark1);
            lcDark2.setMask(lcDark2);
            lcDark3.setMask(lcDark3);
            lcLight1.setMask(lcLight1);
        }
        p.setPen(g.dark());
        p.drawPixmap(0, 0, lcDark2);
        p.drawPixmap(0, 0, lcDark1);
        p.setPen(g.mid());
        p.drawPixmap(0, 0, lcDark3);
        p.setPen(g.light());
        p.drawPixmap(0, 0, lcLight1);
    }
}


static void create_pixmaps()
{
    if(pixmaps_created)
        return;
    pixmaps_created = true;

    if(QPixmap::defaultDepth() > 8){
        aUpperGradient = new KPixmap;
        aUpperGradient->resize(32, 18);
        iUpperGradient = new KPixmap;
        iUpperGradient->resize(32, 18);
        KPixmapEffect::gradient(*aUpperGradient,
                                options->color(Options::TitleBar, true).light(130),
                                options->color(Options::TitleBlend, true),
                                KPixmapEffect::VerticalGradient);
        KPixmapEffect::gradient(*iUpperGradient,
                                options->color(Options::TitleBar, false).light(130),
                                options->color(Options::TitleBlend, false),
                                KPixmapEffect::VerticalGradient);
    }
    // buttons
    QColorGroup btnColor(options->colorGroup(Options::ButtonBg, true));
    buttonPix = new QPixmap(14, 15);
    make_button_fx(btnColor, buttonPix);
    buttonPixDown = new QPixmap(14, 15);
    make_button_fx(btnColor, buttonPixDown, true);

    btnColor = options->colorGroup(Options::ButtonBg, false);
    iButtonPix = new QPixmap(14, 15);
    make_button_fx(btnColor, iButtonPix);
    iButtonPixDown = new QPixmap(14, 15);
    make_button_fx(btnColor, iButtonPixDown, true);


    if(qGray(btnColor.background().rgb()) < 150)
        buttonFg = Qt::white;
    else
        buttonFg = Qt::black;
}

ModernButton::ModernButton(Client *parent, const char *name,
                           const unsigned char *bitmap)
    : QButton(parent, name)
{
    QBitmap mask(14, 15, QPixmap::defaultDepth() > 8 ?
                 btnhighcolor_mask_bits : lowcolor_mask_bits, true);
    resize(14, 15);

    if(bitmap)
        setBitmap(bitmap);
    setMask(mask);
    client = parent;
}

QSize ModernButton::sizeHint() const
{
    return(QSize(14, 15));
}

void ModernButton::reset()
{
    repaint(false);
}

void ModernButton::setBitmap(const unsigned char *bitmap)
{
    deco = QBitmap(8, 8, bitmap, true);
    deco.setMask(deco);
    repaint();
}

void ModernButton::drawButton(QPainter *p)
{
    if(client->isActive()){
        if(buttonPix)
            p->drawPixmap(0, 0, isDown() ? *buttonPixDown : *buttonPix);
    }
    else{
        if(iButtonPix)
            p->drawPixmap(0, 0, isDown() ? *iButtonPixDown : *iButtonPix);
    }
    if(!deco.isNull()){
        p->setPen(buttonFg);
        p->drawPixmap(isDown() ? 4 : 3, isDown() ? 5 : 4, deco);
    }
}

void ModernButton::mousePressEvent( QMouseEvent* e )
{
    last_button = e->button();
    QMouseEvent me ( e->type(), e->pos(), e->globalPos(), LeftButton, e->state() );
    QButton::mousePressEvent( &me );
}

void ModernButton::mouseReleaseEvent( QMouseEvent* e )
{
    QMouseEvent me ( e->type(), e->pos(), e->globalPos(), LeftButton, e->state() );
    QButton::mouseReleaseEvent( &me );
}


void ModernSys::slotReset()
{
    if(aUpperGradient){
        delete aUpperGradient;
        delete iUpperGradient;
    }
    delete buttonPix;
    delete buttonPixDown;
    delete iButtonPix;
    delete iButtonPixDown;

    pixmaps_created = false;
    create_pixmaps();
    titleBuffer.resize(0, 0);
    recalcTitleBuffer();
    button[0]->reset();
    button[1]->reset();
    button[2]->reset();
    button[3]->reset();
    if(button[4])
        button[4]->reset();
    repaint();
}

ModernSys::ModernSys( Workspace *ws, WId w, QWidget *parent,
                            const char *name )
    : Client( ws, w, parent, name, WResizeNoErase )
{
    create_pixmaps();
    connect(options, SIGNAL(resetClients()), this, SLOT(slotReset()));
    bool help = providesContextHelp();

    QGridLayout* g = new QGridLayout(this, 0, 0, 2);
    g->addWidget(windowWrapper(), 1, 1 );
    g->setRowStretch(1, 10);
    g->addItem( new QSpacerItem( 0, 0, QSizePolicy::Fixed, QSizePolicy::Expanding ) );

    g->addColSpacing(0, 2);
    g->addColSpacing(2, 8);

    g->addRowSpacing(2, 8);

    button[0] = new ModernButton(this, "close"/*, close_bits*/);
    button[1] = new ModernButton(this, "sticky");
    if(isSticky())
        button[1]->setBitmap(unsticky_bits);
    else
        button[1]->setBitmap(sticky_bits);
    button[2] = new ModernButton(this, "iconify", iconify_bits);
    button[3] = new ModernButton(this, "maximize", maximize_bits);
    if(help){
        button[4] = new ModernButton(this, "help", question_bits);
        connect( button[4], SIGNAL( clicked() ), this, ( SLOT( contextHelp() ) ) );
    }
    else
        button[4] = NULL;

    connect( button[0], SIGNAL( clicked() ), this, ( SLOT( closeWindow() ) ) );
    connect( button[1], SIGNAL( clicked() ), this, ( SLOT( toggleSticky() ) ) );
    connect( button[2], SIGNAL( clicked() ), this, ( SLOT( iconify() ) ) );
    connect( button[3], SIGNAL( clicked() ), this, ( SLOT( maxButtonClicked() ) ) );

    QHBoxLayout* hb = new QHBoxLayout(0);
    hb->setResizeMode(QLayout::FreeResize);
    g->addLayout( hb, 0, 1 );
    //hb->addSpacing(3);
    hb->addWidget( button[0]);
    titlebar = new QSpacerItem(10, 16, QSizePolicy::Expanding,
                               QSizePolicy::Minimum);
    hb->addSpacing(3);
    hb->addItem(titlebar);
    hb->addSpacing(3);
    if(help){
        hb->addWidget( button[4]);
        hb->addSpacing(1);
    }
    hb->addWidget( button[1]);
    hb->addSpacing(1);
    hb->addWidget( button[2]);
    hb->addSpacing(1);
    hb->addWidget( button[3]);
    hb->addSpacing(2);

    setBackgroundMode(NoBackground);
    recalcTitleBuffer();

}

void ModernSys::maxButtonClicked( )
{
    switch ( button[3]->last_button ) {
    case MidButton:
       maximize( MaximizeVertical );
       break;
    case RightButton:
       maximize( MaximizeHorizontal );
       break;
    default: //LeftButton:
       maximize( MaximizeFull );
       break;
    }
}



void ModernSys::resizeEvent( QResizeEvent* )
{
    //Client::resizeEvent( e );
    recalcTitleBuffer();
    doShape();
    /*
    if ( isVisibleToTLW() && !testWFlags( WNorthWestGravity )) {
        QPainter p( this );
	QRect t = titlebar->geometry();
	t.setTop( 0 );
	QRegion r = rect();
	r = r.subtract( t );
	p.setClipRegion( r );
	p.eraseRect( rect() );
        }*/
}

void ModernSys::recalcTitleBuffer()
{
    if(oldTitle == caption() && width() == titleBuffer.width())
        return;
    QFontMetrics fm(options->font(true));
    titleBuffer.resize(width(), 18);
    QPainter p;
    p.begin(&titleBuffer);
    if(aUpperGradient)
        p.drawTiledPixmap(0, 0, width(), 18, *aUpperGradient);
    else
        p.fillRect(0, 0, width(), 18,
                   options->colorGroup(Options::TitleBar, true).
                   brush(QColorGroup::Button));

    QRect t = titlebar->geometry();
    t.setTop( 2 );
    t.setLeft( t.left() + 4 );
    t.setRight( t.right() - 2 );

    QRegion r(t.x(), 0, t.width(), 18);
    r -= QRect(t.x()+((t.width()-fm.width(caption()))/2)-4,
               0, fm.width(caption())+8, 18);
    p.setClipRegion(r);
    int i, ly;
    for(i=0, ly=4; i < 4; ++i, ly+=3){
        p.setPen(options->color(Options::TitleBar, true).light(150));
        p.drawLine(0, ly, width()-1, ly);
        p.setPen(options->color(Options::TitleBar, true).dark(120));
        p.drawLine(0, ly+1, width()-1, ly+1);
    }
    p.setClipRect(t);
    p.setPen(options->color(Options::Font, true));
    p.setFont(options->font(true));

    p.drawText(t.x()+((t.width()-fm.width(caption()))/2)-4,
               0, fm.width(caption())+8, 18, AlignCenter, caption());
    p.setClipping(false);
    p.end();
    oldTitle = caption();
}

void ModernSys::captionChange( const QString &)
{
    recalcTitleBuffer();
    repaint( titlebar->geometry(), false );
}

void ModernSys::drawRoundFrame(QPainter &p, int x, int y, int w, int h)
{
    kDrawRoundButton(&p, x, y, w, h,
                     options->colorGroup(Options::Frame, isActive()), false);

}

void ModernSys::paintEvent( QPaintEvent* )
{
    QPainter p( this );
    QRect t = titlebar->geometry();

    QBrush fillBrush(colorGroup().brush(QColorGroup::Background).pixmap() ?
                     colorGroup().brush(QColorGroup::Background) :
                     options->colorGroup(Options::Frame, isActive()).
                     brush(QColorGroup::Button));

    p.fillRect(1, 16, width()-2, height()-16, fillBrush);
    p.fillRect(width()-6, 0, width()-1, height(), fillBrush);

    t.setTop( 2 );
    t.setLeft( t.left() + 4 );
    t.setRight( t.right() - 2 );

    int w = width()-6; // exclude handle
    int h = height()-6;

    // titlebar
    QColorGroup g = options->colorGroup(Options::TitleBar, isActive());
    if(isActive()){
        p.drawPixmap(1, 1, titleBuffer, 0, 0, w-2, 18);
    }
    else{
        if(iUpperGradient)
            p.drawTiledPixmap(1, 1, w-2, 18, *iUpperGradient);
        else
            p.fillRect(1, 1, w-2, 18, fillBrush);
        p.setPen(options->color(Options::Font, isActive()));
        p.setFont(options->font(isActive()));
        p.drawText(t, AlignCenter, caption() );
    }

    // titlebar highlight
    p.setPen(g.light());
    p.drawLine(1, 1, 1, 19);
    p.drawLine(1, 1, w-3, 1);
    p.setPen(g.dark());
    p.drawLine(w-2, 1, w-2, 19);
    p.drawLine(0, 18, w-2, 18);

    // frame
    g = options->colorGroup(Options::Frame, isActive());
    p.setPen(g.light());
    p.drawLine(1, 19, 1, h-2);
    p.setPen(g.dark());
    p.drawLine(2, h-2, w-2, h-2);
    p.drawLine(w-2, 19, w-2, h-2);
    //p.drawPoint(w-3, 19);
    //p.drawPoint(2, 19);

    qDrawShadePanel(&p, 3, 19, w-6, h-22, g, true);

    // handle
    p.setPen(g.dark());
    p.drawLine(width()-3, height()-29, width()-3, height()-3);
    p.drawLine(width()-29, height()-3, width()-3, height()-3);

    p.setPen(g.light());
    p.drawLine(width()-6, height()-29, width()-6, height()-6);
    p.drawLine(width()-29, height()-6, width()-6, height()-6);
    p.drawLine(width()-6, height()-29, width()-4, height()-29);
    p.drawLine(width()-29, height()-6, width()-29, height()-4);


    p.setPen(Qt::black);
    p.drawRect(0, 0, w, h);

    // handle outline
    p.drawLine(width()-6, height()-30, width(), height()-30);
    p.drawLine(width()-2, height()-30, width()-2, height()-2);
    p.drawLine(width()-30, height()-2, width()-2, height()-2);
    p.drawLine(width()-30, height()-6, width()-30, height()-2);


}

#define QCOORDARRLEN(x) sizeof(x)/(sizeof(QCOORD)*2)

void ModernSys::doShape()
{
    QRegion mask;
    mask += QRect(0, 0, width()-6, height()-6);
    mask += QRect(width()-30, height()-30, 29, 29);
    //single points
    mask -= QRect(0, 0, 1, 1);
    mask -= QRect(width()-7, 0, 1, 1);
    mask -= QRect(0, height()-7, 1, 1);
    mask -= QRect(width()-2, height()-2, 1, 1);
    mask -= QRect(width()-2, height()-30, 1, 1);
    mask -= QRect(width()-30, height()-2, 1, 1);

    setMask(mask);
}

void ModernSys::showEvent(QShowEvent *ev)
{
    Client::showEvent(ev);
    doShape();
    repaint();
}

void ModernSys::windowWrapperShowEvent( QShowEvent* )
{
    doShape();
}

void ModernSys::mouseDoubleClickEvent( QMouseEvent * e )
{
    if (titlebar->geometry().contains( e->pos() ) )
	workspace()->performWindowOperation( this, options->operationTitlebarDblClick() );
    workspace()->requestFocus( this );
}

void ModernSys::stickyChange(bool on)
{
    button[1]->setBitmap(on ? unsticky_bits : sticky_bits);
}

void ModernSys::maximizeChange(bool m)
{
    button[3]->setBitmap(m ? minmax_bits : maximize_bits);
}

void ModernSys::init()
{
    //
}

void ModernSys::activeChange(bool)
{
    repaint(false);
    button[0]->reset();
    button[1]->reset();
    button[2]->reset();
    button[3]->reset();
    if(button[4])
        button[4]->reset();
}


Client::MousePosition ModernSys::mousePosition( const QPoint& p) const
{
    MousePosition m = Client::mousePosition( p );

    if ( m == Center ) {
	int border = 10;
	if ( p.y() >= height()-border )
	    m =  Bottom;
	else if ( p.x() >= width()-border )
	    m =  Right;
    }
    return m;
}
#include "modernsys.moc"
