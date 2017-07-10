const scrapeIt = require('scrape-it');
const fs = require('fs');

const url = 'https://developer.mozilla.org/en/docs/Web/HTML/Element';
const parseTag = {
  tags: '.standard-table a code'
};
const blacklistTags = ['h1–h6'];

const letsScrape = async () => {
  // Scrape data
  const data = await scrapeIt(url, parseTag);

  // Write header
  let output = '#ifndef SEE_PHIT_TAGS_HPP\n#define SEE_PHIT_TAGS_HPP\n\n';
  output += 'constexpr const char *tags[] = {\n';

  // Write comma seperated html tags
  data.tags
    .split('>')
    .map(tag => tag.split('<').pop())
    .filter(String)
    .filter(i => !blacklistTags.includes(i))
    .filter((x, i, a) => a.indexOf(x) === i)
    .concat(['h1', 'h2', 'h3', 'h4', 'h5', 'h6'])
    .sort()
    .forEach(tag => {
      output += '  "' + tag + '",\n';
    });

  // Write footer
  output += '};\n\n';
  output += '#endif\n';

  // Write all to file
  fs.writeFileSync('tags.hpp', output);
};

letsScrape();
