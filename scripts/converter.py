from optparse import OptionParser

def display_value(b):
    print('\n\t' + '-'*25)
    print('Bytes     : {}'.format(b))
    print('Kilobytes : {}'.format(b / 1024.0))
    print('Megabytes : {}'.format(b / 1048576.0))
    print('Gigabytes : {}'.format(b / 1073741824.0))
    print('\t' + '-'*25)

if __name__ == '__main__':
    parser = OptionParser()
    parser.add_option('-b', '--bytes', dest='bytes')
    parser.add_option('-k', '--kilobytes', dest='kb')
    parser.add_option('-m', '--megabytes', dest='mb')
    parser.add_option('-g', '--gigabytes', dest='gb')
    (options, args) = parser.parse_args()
    if options.bytes:
        display_value(int(options.bytes, 0))
    if options.kb:
        display_value(int(options.kb, 0) * 1024.0)
    if options.mb:
        display_value(int(options.mb, 0) * 1048576.0)
    if options.gb:
        display_value(int(options.gb, 0) * 1073741824.0)
