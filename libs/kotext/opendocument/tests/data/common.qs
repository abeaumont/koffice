var document = new QTextDocument;
var cursor = new QTextCursor(document);

// See KoCharacterStyle.cpp
var defaultFont = new QFont;
defaultFont.setPointSizeF(12.0);
defaultFont.setWeight(QFont.Normal);

var defaultTextFormat = new QTextCharFormat;
defaultTextFormat.setFont(defaultFont);
defaultTextFormat.setVerticalAlignment(QTextCharFormat.AlignNormal);

QTextCharFormat.clone = function(fmt) {
    var newFormat = new QTextCharFormat;
    // FIXME: What we really need to do is -> newFormat.properties = fmt.properties;

    newFormat.setFont(fmt.font());
    // CHECKME: When new formats are created, the color is explicitly set to Black
    if (newFormat.foreground().style() == 0) {
        newFormat.setForeground(new QBrush(new QColor(0, 0, 0)));
    }
    return newFormat;
};

// KOffice specific
var KoListStyle = {};
KoListStyle.StartValue = 0x1003EA;
KoListStyle.Level = 0x1003EB;

var defaultListItemFormat = new QTextCharFormat;
defaultListItemFormat.setFont(defaultFont);
defaultListItemFormat.setVerticalAlignment(QTextCharFormat.AlignNormal);
//defaultListItemFormat.setProperty(0x1003EA, 1);
//defaultListItemFormat.setProperty(0x1003EB, 1);

