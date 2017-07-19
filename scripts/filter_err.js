function filterError(errString='') {
  let pattern = /(?:ParseError\(.*"(.+)"\))[\s\S]*(?:error: array subscript value '([0-9]+)')/;
  let groups = errString.match(pattern);
  if (groups) {
    return `${groups[1]} at line: ${groups[2]}`;
  }
  return errString;
}

if (!process.argv[2])
  return console.error('Usage: node filter_err <str>');

console.log(filterError(process.argv[2]));
