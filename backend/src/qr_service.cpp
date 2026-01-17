//
// Credit: Anon from StackOverflow
//

#include "qr_service.hpp"
#include "qrcodegen.hpp"
#include <sstream>

namespace fastdrop {

    std::string generate_qr_svg(const std::string& data) {
        using namespace qrcodegen;
        
        QrCode qr = QrCode::encodeText(data.c_str(), QrCode::Ecc::MEDIUM);
        
        int size = qr.getSize();
        int border = 2;
        
        std::ostringstream svg;
        svg << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
        svg << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\" ";
        svg << "viewBox=\"0 0 " << (size + border * 2) << " " << (size + border * 2) << "\" ";
        svg << "stroke=\"none\">\n";
        svg << "<rect width=\"100%\" height=\"100%\" fill=\"#FFFFFF\"/>\n";
        svg << "<path d=\"";
        
        for (int y = 0; y < size; y++) {
            for (int x = 0; x < size; x++) {
                if (qr.getModule(x, y)) {
                    svg << "M" << (x + border) << "," << (y + border) << "h1v1h-1z";
                }
            }
        }
        
        svg << "\" fill=\"#000000\"/>\n";
        svg << "</svg>\n";
        
        return svg.str();
    }

}
